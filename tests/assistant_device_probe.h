/* Diagnostic firmware only: synthetic silence, no user microphone upload. */
#include "esp_attr.h"
static RTC_DATA_ATTR uint32_t assistant_reboot_id;
static void assistant_device_probe_tick(void){
    static unsigned phase;static int64_t next=15000000,deadline,stable;
    static uint32_t chime_id;
    int64_t now=esp_timer_get_time();if(phase==99||now<next)return;next=now+250000;badge_power_activity();
    if(phase==0&&assistant_reboot_id){
        cJSON *o=badge_alarm_service_json(),*list=cJSON_GetObjectItemCaseSensitive(o,"reminders");bool found=false;
        for(cJSON *v=list->child;v;v=v->next)if((uint32_t)cJSON_GetObjectItemCaseSensitive(v,"id")->valueint==assistant_reboot_id)found=true;
        cJSON_Delete(o);configASSERT(found&&esp_sleep_get_wakeup_cause()==ESP_SLEEP_WAKEUP_TIMER);
        configASSERT(badge_alarm_service_cancel(assistant_reboot_id));assistant_reboot_id=0;
        ESP_LOGI("assistant_probe","PERSISTENCE_AND_DEEP_SLEEP_PASS");ESP_LOGI("assistant_probe","DONE");phase=99;return;
    }
    if(phase==0){
        if(badge_alarm_service_now()<BADGE_CLOCK_MIN){configASSERT(now<90000000);return;}
        configASSERT(control_home());input_event_t e={BSP_BTN_UP,BSP_BTN_LONG};process_input(&e);
        deadline=now+30000000;phase=1;
    }else if(phase==1){
        configASSERT(!demo_xiaozhi_connect_probe(1)&&now<deadline);
        if(demo_xiaozhi_connect_probe(0)!=4)return;
        if(!stable)stable=now;
        if(now-stable<4000000)return;
        cJSON *o=profile_edit_get(5);configASSERT(o);
        unsigned id=cJSON_GetObjectItemCaseSensitive(o,"number")->valueint-1,rev=cJSON_GetObjectItemCaseSensitive(o,"revision")->valueint;
        const char *title=cJSON_GetObjectItemCaseSensitive(o,"title")->valuestring;
        configASSERT(profile_protocol_edit(id,rev,2,title)==ESP_OK); /* No mutation. */
        cJSON_Delete(o);ESP_LOGI("assistant_probe","PROFILE_READ_AND_NOOP_EDIT_PASS");
        badge_control_command_t c={.kind=BC_REMINDER_SET,.value=3};snprintf(c.text,sizeof(c.text),"测试提醒");
        configASSERT(execute_control(c));chime_id=badge_control_created_id();configASSERT(chime_id);phase=2;deadline=now+15000000;
    }else if(phase==2){
        configASSERT(now<deadline);if(!reminder.ringing)return;
        configASSERT(reminder_current_id==chime_id);next=now+2500000;phase=3;
    }else if(phase==3){
        input_event_t e={BSP_BTN_OK,BSP_BTN_CLICK};process_input(&e);phase=4;
    }else if(phase==4){
        if(reminder.ringing)return;
        ESP_LOGI("assistant_probe","PERSISTENT_CHIME_AND_ACK_PASS");
        int64_t due=badge_alarm_service_now()+600;
        configASSERT(badge_alarm_service_add(due,0,0,"重启恢复测试",&assistant_reboot_id));
        ESP_LOGI("assistant_probe","ENTER_DEEP_SLEEP");control_shutdown();configASSERT(false);
    }
}
