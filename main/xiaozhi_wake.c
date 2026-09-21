#include "xiaozhi_wake.h"
#ifdef BADGE_WAKE_HOST_TEST
#include "xiaozhi_wake_test_stubs.h"
#else
#include "badge_power.h"
#include "bsp_audio.h"
#include "esp_wn_models.h"
#include "model_path.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#endif
#include <stdatomic.h>
#include <stdlib.h>
#ifndef BADGE_WAKE_HOST_TEST
#include "wake_model_config.h"
#endif

#ifdef BADGE_WAKE_HOST_TEST
static const unsigned char wake_start[]={0};
#else
extern const unsigned char wake_start[] asm("_binary_wake_model_bin_start");
#endif
static atomic_bool enabled,stop_requested,trigger;
static atomic_uint status;
static SemaphoreHandle_t done;
static bool running;
static TaskHandle_t worker_task;
static atomic_bool finished;
static int64_t retry_at;
static void (*notify_trigger)(void);
static void (*prepare_memory)(void);
void xz_wake_on_prepare(void (*prepare)(void)){prepare_memory=prepare;}
void xz_wake_on_trigger(void (*notify)(void)){notify_trigger=notify;}
#ifdef BADGE_WAKE_DEVICE_PROBE
static atomic_uint probe_frames;
static atomic_bool probe_trigger,probe_fail_once;
void xz_wake_probe_fail_once(void){atomic_store(&probe_fail_once,true);}
unsigned xz_wake_probe_frames(void){return atomic_load(&probe_frames);}
void xz_wake_probe_trigger(void){atomic_store(&probe_trigger,true);}
#endif
enum { WAKE_OFF, WAKE_LOADING, WAKE_LISTENING, WAKE_SUSPENDED, WAKE_ERROR };
const char *xz_wake_phrase(void){return BADGE_WAKE_PHRASE;}
const char *xz_wake_status(void){
    static const char *const names[]={"后台唤醒已关闭","加载唤醒模型","等待语音唤醒","音频占用，唤醒暂停","唤醒失败，稍后自动重试"};
    return names[atomic_load(&status)];
}
bool xz_wake_enabled(void){return atomic_load(&enabled);}
bool xz_wake_listening(void){return atomic_load(&status)==WAKE_LISTENING&&!atomic_load(&finished);}
void xz_wake_init(void){
    nvs_handle_t n;uint8_t value=0;
    if(nvs_open("xiaozhi",NVS_READONLY,&n)==ESP_OK){nvs_get_u8(n,"wake_enabled",&value);nvs_close(n);}
    atomic_store(&enabled,value==1);atomic_store(&status,value==1?WAKE_SUSPENDED:WAKE_OFF);
    ESP_LOGI("xz_wake","Preference enabled=%u",value==1);
}
static void worker(void *arg){
    (void)arg;bool failed=false,power_owned=false;int16_t *pcm=NULL;model_iface_data_t *model=NULL;
    srmodel_list_t *list=NULL;const esp_wn_iface_t *engine=NULL;
#ifdef BADGE_WAKE_DEVICE_PROBE
    if(atomic_exchange(&probe_fail_once,false)){ESP_LOGW("xz_wake","INJECT_START_FAILURE");failed=true;goto cleanup;}
#endif
    if(esp_get_free_heap_size()<49152||heap_caps_get_largest_free_block(MALLOC_CAP_8BIT)<32768){
        ESP_LOGW("xz_wake","Memory gate free=%lu largest=%lu",(unsigned long)esp_get_free_heap_size(),(unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
        failed=true;goto cleanup;
    }
    list=srmodel_load(wake_start);
    engine=list?esp_wn_handle_from_name(list->model_name[0]):NULL;
    if(engine)model=engine->create(list->model_name[0],DET_MODE_90);
    if(model&&BADGE_WAKE_THRESHOLD>0)engine->set_det_threshold(model,BADGE_WAKE_THRESHOLD,1);
    int samples=model?engine->get_samp_chunksize(model):0;
    if(samples>0&&samples<=4096)pcm=calloc((size_t)samples,sizeof(int16_t));
    if(!pcm||!model||engine->get_samp_rate(model)!=16000)failed=true;
    badge_power_enter();power_owned=true;badge_power_set_local_listening(true);
    if(!failed&&(bsp_audio_init()!=ESP_OK||bsp_audio_set_format(16000,16,1)!=ESP_OK))failed=true;
    int64_t guard=esp_timer_get_time()+750000;
    if(!failed)atomic_store(&status,WAKE_LISTENING);
    ESP_LOGI("xz_wake","Detector %s free=%lu",failed?"failed":"ready",(unsigned long)esp_get_free_heap_size());
    while(!failed&&!atomic_load(&stop_requested)){
        badge_power_tick(false,true);
        esp_err_t audio_result=bsp_audio_read(pcm,(size_t)samples*2);
        if(audio_result!=ESP_OK){ESP_LOGW("xz_wake","Microphone read failed: %s",esp_err_to_name(audio_result));failed=true;break;}
        int result=engine->detect(model,pcm);
#ifdef BADGE_WAKE_DEVICE_PROBE
        atomic_fetch_add(&probe_frames,1);
        result=atomic_exchange(&probe_trigger,false)?1:0;
#endif
        if(result>0&&esp_timer_get_time()>guard){ESP_LOGI("xz_wake","Wake detected");atomic_store(&trigger,true);if(notify_trigger)notify_trigger();break;}
        taskYIELD();
    }
cleanup:
    /* Only this worker touches the detector; join precedes audio handover. */
    free(pcm);if(model)engine->destroy(model);if(list)esp_srmodel_deinit(list);
    if(power_owned){badge_power_set_local_listening(false);(void)badge_power_leave();}
    if(failed)atomic_store(&status,WAKE_ERROR);
    atomic_store(&finished,true);xSemaphoreGive(done);
    /* Joiner frees the 8 KiB stack synchronously before voice startup. */
    for(;;)vTaskSuspend(NULL);
}
esp_err_t xz_wake_stop(void){
    atomic_store(&stop_requested,true);
    if(running){if(xSemaphoreTake(done,pdMS_TO_TICKS(3000))!=pdTRUE)return ESP_ERR_TIMEOUT;vTaskDelete(worker_task);worker_task=NULL;running=false;atomic_store(&finished,false);}
    if(done){vSemaphoreDelete(done);done=NULL;}
    atomic_store(&trigger,false);
    if(atomic_load(&status)!=WAKE_ERROR)atomic_store(&status,xz_wake_enabled()?WAKE_SUSPENDED:WAKE_OFF);
    retry_at=esp_timer_get_time()+750000;return ESP_OK;
}
esp_err_t xz_wake_enable(bool value){
    if(!value){esp_err_t e=xz_wake_stop();if(e!=ESP_OK)return e;}
    nvs_handle_t n;esp_err_t e=nvs_open("xiaozhi",NVS_READWRITE,&n);
    if(e==ESP_OK){e=nvs_set_u8(n,"wake_enabled",value);if(e==ESP_OK)e=nvs_commit(n);nvs_close(n);}
    if(e==ESP_OK){
        atomic_store(&enabled,value);
        if(!value||!running||atomic_load(&finished))atomic_store(&status,value?WAKE_SUSPENDED:WAKE_OFF);
        retry_at=0;ESP_LOGI("xz_wake","Preference enabled=%u",value);
    }
    return e;
}
bool xz_wake_take_trigger(void){return atomic_exchange(&trigger,false);}
void xz_wake_tick(bool available){
    if(!available||!xz_wake_enabled())return;
    if(running){
        if(!atomic_load(&finished)||atomic_load(&trigger))return;
        /* A finished failed worker is still owned until joined. Do not latch
         * running/error forever, and do not leak its stack on each retry. */
        if(xz_wake_stop()!=ESP_OK)return;
        atomic_store(&status,WAKE_ERROR);retry_at=esp_timer_get_time()+3000000;
        ESP_LOGW("xz_wake","Detector stopped; retry in 3 seconds");
    }
    if(esp_timer_get_time()<retry_at)return;
    /* Cached cloud transports are optional; local detection needs one large
     * model allocation in addition to its 8 KiB worker stack. */
    if(prepare_memory&&(esp_get_free_heap_size()<65536||heap_caps_get_largest_free_block(MALLOC_CAP_8BIT)<40960)){
        ESP_LOGI("xz_wake","Reclaim idle connection before detector start");prepare_memory();
    }
    retry_at=esp_timer_get_time()+3000000;
    done=xSemaphoreCreateBinary();if(!done){atomic_store(&status,WAKE_ERROR);return;}
    atomic_store(&finished,false);atomic_store(&stop_requested,false);atomic_store(&status,WAKE_LOADING);
    if(xTaskCreate(worker,"xz_wake",8192,NULL,4,&worker_task)!=pdPASS){
        ESP_LOGW("xz_wake","Task allocation failed free=%lu largest=%lu",(unsigned long)esp_get_free_heap_size(),(unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
        worker_task=NULL;vSemaphoreDelete(done);done=NULL;atomic_store(&status,WAKE_ERROR);return;
    }
    running=true;
}
