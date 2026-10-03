#include "badge_power.h"
#include "badge_power_logic.h"
#include "badge_network.h"
#include "bsp_display.h"
#include "bsp_audio.h"
#include "esp_pm.h"
#include "esp_timer.h"
#include "esp_log.h"
#include <stdatomic.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static const char *TAG="badge_power";
static atomic_bool enabled,screen_off,wake_requested,worker_busy,local_listening;
static atomic_uint last_activity;
static uint32_t last_audio,last_retry,display_retry;
static unsigned timeout_choice=BADGE_SCREEN_TIMEOUT_DEFAULT;
static bool audio_was_dark;
static SemaphoreHandle_t audio_transition;
static uint32_t orphan_retry;
static esp_pm_lock_handle_t cpu_lock,sleep_lock,audio_cpu_lock,audio_sleep_lock;
static bool pm_ready;
static atomic_bool locks_held;
/* Audio lifecycle is joined before another worker becomes the owner. These
 * leases are separate from the shell's display leases: waking audio must not
 * wait for the shell's next (up to one second) timer tick. */
static atomic_bool audio_locks_held;
static void audio_performance(bool full){
    if(!pm_ready||full==atomic_load(&audio_locks_held))return;
    if(full){esp_pm_lock_acquire(audio_cpu_lock);esp_pm_lock_acquire(audio_sleep_lock);badge_network_standby(false);}
    else{esp_pm_lock_release(audio_sleep_lock);esp_pm_lock_release(audio_cpu_lock);}
    atomic_store(&audio_locks_held,full);
}
void badge_power_set_local_listening(bool value){atomic_store(&local_listening,value);}
static uint32_t now_ms(void){return (uint32_t)(esp_timer_get_time()/1000);}
static void performance(bool full){
    if(!pm_ready||full==locks_held)return;
    if(full){esp_pm_lock_acquire(cpu_lock);esp_pm_lock_acquire(sleep_lock);}
    else{esp_pm_lock_release(sleep_lock);esp_pm_lock_release(cpu_lock);}
    locks_held=full;
}
esp_err_t badge_power_init(void){
    if(pm_ready)return ESP_OK;
    if(!audio_transition)audio_transition=xSemaphoreCreateMutex();
    if(!audio_transition)return ESP_ERR_NO_MEM;
    esp_err_t e=esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX,0,"badge_active",&cpu_lock);
    if(e!=ESP_OK)return e;
    e=esp_pm_lock_create(ESP_PM_NO_LIGHT_SLEEP,0,"badge_display",&sleep_lock);
    if(e!=ESP_OK){esp_pm_lock_delete(cpu_lock);cpu_lock=NULL;return e;}
    e=esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX,0,"badge_audio",&audio_cpu_lock);
    if(e==ESP_OK)e=esp_pm_lock_create(ESP_PM_NO_LIGHT_SLEEP,0,"badge_audio",&audio_sleep_lock);
    if(e!=ESP_OK){if(audio_cpu_lock)esp_pm_lock_delete(audio_cpu_lock);esp_pm_lock_delete(sleep_lock);esp_pm_lock_delete(cpu_lock);audio_cpu_lock=NULL;sleep_lock=NULL;cpu_lock=NULL;return e;}
    esp_pm_lock_acquire(cpu_lock);esp_pm_lock_acquire(sleep_lock);locks_held=true;
    const esp_pm_config_t cfg={.max_freq_mhz=160,.min_freq_mhz=40,.light_sleep_enable=true};
    e=esp_pm_configure(&cfg);pm_ready=e==ESP_OK;
    if(e!=ESP_OK){
        esp_pm_lock_release(sleep_lock);esp_pm_lock_release(cpu_lock);locks_held=false;
        esp_pm_lock_delete(audio_sleep_lock);esp_pm_lock_delete(audio_cpu_lock);
        esp_pm_lock_delete(sleep_lock);esp_pm_lock_delete(cpu_lock);
        audio_sleep_lock=audio_cpu_lock=sleep_lock=cpu_lock=NULL;
    }
    ESP_LOGI(TAG,"DFS 160/40 MHz, automatic light sleep: %s",esp_err_to_name(e));
    return e;
}
bool badge_power_screen_off(void){return atomic_load(&screen_off);}
badge_power_status_t badge_power_status(void){
    return (badge_power_status_t){.screen_off=badge_power_screen_off(),.configured=pm_ready,
        .display_performance=atomic_load(&locks_held),.audio_performance=atomic_load(&audio_locks_held),
        .audio_owner=atomic_load(&enabled),.voice_listening=atomic_load(&local_listening),
        .audio_sleeping=bsp_audio_is_sleeping()};
}
void badge_power_activity(void){
    atomic_store(&last_activity,now_ms());atomic_store(&wake_requested,true);
}
void badge_power_set_timeout(unsigned choice){
    timeout_choice=choice<BADGE_SCREEN_TIMEOUT_COUNT?choice:BADGE_SCREEN_TIMEOUT_DEFAULT;
    badge_power_activity();
}
void badge_power_audio_activity(void){last_audio=now_ms();}
static bool wake_display(void){
    performance(true);
    esp_err_t e=bsp_display_resume();
    if(e!=ESP_OK){ESP_LOGW(TAG,"Display resume retry: %s",esp_err_to_name(e));return false;}
    atomic_store(&screen_off,false);
    ESP_LOGI(TAG,"Display awake, brightness=%u",bsp_display_brightness());
    return true;
}
void badge_power_enter(void){
    if(audio_transition)xSemaphoreTake(audio_transition,portMAX_DELAY);
    last_audio=now_ms();last_retry=last_audio-1000;audio_was_dark=false;
    audio_performance(true);
    atomic_store(&worker_busy,false);atomic_store(&enabled,true);
    if(audio_transition)xSemaphoreGive(audio_transition);
}
esp_err_t badge_power_leave(void){
    /* Join must not wait forever for damaged hardware. Hand a failed suspend
     * to the shell, which retries only while no new audio owner exists. */
    if(audio_transition)xSemaphoreTake(audio_transition,portMAX_DELAY);
    esp_err_t e=bsp_audio_sleep();
    if(e!=ESP_OK)ESP_LOGW(TAG,"Audio exit suspend pending: %s",esp_err_to_name(e));
    audio_performance(!bsp_audio_is_sleeping());
    atomic_store(&enabled,false);atomic_store(&worker_busy,false);
    atomic_store(&local_listening,false);badge_network_xiaozhi_power(false,false);
    orphan_retry=now_ms();
    if(audio_transition)xSemaphoreGive(audio_transition);
    return e;
}
static void retry_unowned_audio(void){
    if(!audio_transition||xSemaphoreTake(audio_transition,0)!=pdTRUE)return;
    uint32_t now=now_ms();
    if(!atomic_load(&enabled)&&!bsp_audio_is_sleeping()&&now-orphan_retry>=1000){
        orphan_retry=now;
        esp_err_t e=bsp_audio_sleep();
        audio_performance(!bsp_audio_is_sleeping());
        if(e!=ESP_OK)ESP_LOGW(TAG,"Unowned audio suspend retry: %s",esp_err_to_name(e));
    }
    xSemaphoreGive(audio_transition);
}
void badge_power_audio_tick(bool audio_busy){
    if(!atomic_load(&enabled))return;
    uint32_t now=now_ms();
    if(audio_busy)audio_performance(true);
    if(audio_busy&&bsp_audio_needs_wake()){
        if(bsp_audio_wake()!=ESP_OK)return;
        last_audio=now_ms();
    }
    if(audio_busy)last_audio=now;
    if(badge_idle_due(now,last_audio,BADGE_AUDIO_IDLE_MS,audio_busy)&&!bsp_audio_is_sleeping()&&now-last_retry>=1000){
        last_retry=now;esp_err_t e=bsp_audio_sleep();
        if(e!=ESP_OK)ESP_LOGW(TAG,"Audio idle suspend: %s",esp_err_to_name(e));
        else ESP_LOGI(TAG,"Audio idle: ES8311 suspended, I2S input/output stopped");
    }
    audio_performance(audio_busy||!bsp_audio_is_sleeping());
}
void badge_power_tick(bool busy,bool audio_busy){
    if(!atomic_load(&enabled))return;
    atomic_store(&worker_busy,busy);
    if(busy)atomic_store(&last_activity,now_ms());
    bool dark=badge_power_screen_off();
    badge_power_audio_tick(audio_busy||(audio_was_dark&&!dark));
    audio_was_dark=dark;
    badge_network_xiaozhi_power(true,busy);
}
/* One display owner for every page. Audio workers never stop/resume LVGL. */
void badge_power_display_tick(bool keep_awake,bool keep_cpu){
    retry_unowned_audio();
    uint32_t now=now_ms();
    bool busy=keep_awake||atomic_load(&worker_busy);
    bool requested=atomic_exchange(&wake_requested,false);
    uint32_t delay=badge_screen_timeout_ms(timeout_choice);
    if(busy)atomic_store(&last_activity,now);
    if(badge_power_screen_off()&&(requested||busy||delay==0)){
        if(!wake_display())atomic_store(&wake_requested,true);
        else badge_network_wake_probe();
    }
    if(!badge_power_screen_off()&&badge_idle_due(now,atomic_load(&last_activity),delay,busy)&&now-display_retry>=1000){
        display_retry=now;
        atomic_store(&screen_off,true); // Suppress status/UI writes during transition.
        esp_err_t e=bsp_display_suspend();
        if(e!=ESP_OK){
            atomic_store(&screen_off,bsp_display_is_suspended());
            atomic_store(&wake_requested,true);performance(true);
            ESP_LOGW(TAG,"Display suspend, recovering: %s",esp_err_to_name(e));return;
        }
        ESP_LOGI(TAG,"Display asleep: backlight=0, panel sleep, LVGL task/tick stopped");
    }
    bool listening=atomic_load(&local_listening);
    bool audio_full=atomic_load(&audio_locks_held);
    performance(!badge_power_screen_off()||keep_cpu||busy||listening);
    badge_network_standby(badge_power_screen_off()&&!keep_cpu&&!busy&&!listening&&!audio_full);
}
