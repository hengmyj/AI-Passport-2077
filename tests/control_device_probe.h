/* Opt-in bench probe, executed exclusively by the navigation task. Never
 * sends microphone recordings or changes badge images/Wi-Fi credentials. */
#include "xiaozhi_tools.h"
#include "voice_catalog.h"
#include "esp_system.h"
extern unsigned demo_xiaozhi_probe_volume(void);
static uint8_t saved_radio[3];static bool saved_radio_present[3],saved_radio_name_present;
static char saved_radio_name[64];static const char *radio_keys[]={"volume","city","station"};
static void probe_radio_settings(bool restore){
    nvs_handle_t h;configASSERT(nvs_open("badge_radio_v1",NVS_READWRITE,&h)==ESP_OK);
    for(unsigned i=0;i<3;i++){
        if(restore){if(saved_radio_present[i])configASSERT(nvs_set_u8(h,radio_keys[i],saved_radio[i])==ESP_OK);else nvs_erase_key(h,radio_keys[i]);}
        else saved_radio_present[i]=nvs_get_u8(h,radio_keys[i],&saved_radio[i])==ESP_OK;
    }
    if(restore){if(saved_radio_name_present)configASSERT(nvs_set_str(h,"station_name",saved_radio_name)==ESP_OK);else nvs_erase_key(h,"station_name");configASSERT(nvs_commit(h)==ESP_OK);}
    else {size_t len=sizeof(saved_radio_name);saved_radio_name_present=nvs_get_str(h,"station_name",saved_radio_name,&len)==ESP_OK;}
    nvs_close(h);
}
static void control_probe_start_xz(void){
    configASSERT(control_home());
    input_event_t key={BSP_BTN_UP,BSP_BTN_LONG};process_input(&key);
    configASSERT(navigation.page==BADGE_AI_CHAT&&demo_xiaozhi_active());
}
static uint32_t control_probe_call(const char *name,const char *args,bool send){
    char text[320];snprintf(text,sizeof(text),"{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"tools/call\",\"params\":{\"name\":\"%s\",\"arguments\":%s}}",name,args);
    cJSON *q=cJSON_Parse(text);uint32_t ticket=0;cJSON *r=xz_tools_handle(q,40,true,&ticket);
    configASSERT(r&&!cJSON_HasObjectItem(r,"error")&&!cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(cJSON_GetObjectItemCaseSensitive(r,"result"),"isError")));
    cJSON_Delete(q);cJSON_Delete(r);badge_control_commit(ticket,send);return ticket;
}
static void control_probe_tick(void){
    static unsigned stage,original_badge,original_brightness,target_badge,clip;static bool has_qr,original_wake;
    static unsigned original_volume;static bool saw_sleep;static int64_t next=12000000;
    static unsigned homes_before;
    if(stage>41||esp_timer_get_time()<next)return;
    next=esp_timer_get_time()+2000000;char args[512];
    switch(stage){
    case 0:
        original_wake=xz_wake_enabled();configASSERT(xz_wake_enable(false)==ESP_OK);
        probe_radio_settings(false);
        configASSERT(navigation.badge_mask);original_badge=navigation.active_badge;original_brightness=navigation.brightness;
        target_badge=original_badge;for(unsigned i=0;i<navigation.badge_count;i++)if(i!=original_badge&&(navigation.badge_mask&(1u<<i))){target_badge=i;break;}
        has_qr=(control_qr_mask&(1u<<original_badge))!=0;
        for(unsigned i=1;i<VOICE_CLIP_COUNT;i++)if(voice_clips[i].samples<voice_clips[clip].samples)clip=i;
        control_probe_start_xz();break;
    case 1:
        {cJSON *q=cJSON_Parse("{\"jsonrpc\":\"2.0\",\"id\":3,\"method\":\"tools/list\"}");uint32_t t;cJSON *r=xz_tools_handle(q,40,true,&t);configASSERT(r);char *text=cJSON_PrintUnformatted(r);configASSERT(text);ESP_LOGI("control_probe","TOOLS_LIST_PASS bytes=%u",(unsigned)strlen(text));cJSON_free(text);cJSON_Delete(r);cJSON_Delete(q);}
        control_probe_call("self.get_device_status","{}",true);
        control_probe_call("self.audio.search","{\"query\":\"\"}",true);
        control_probe_call("self.display.set_brightness","{\"percent\":20}",false);
        {badge_control_command_t c;configASSERT(!badge_control_take(&c));}
        control_probe_call("self.display.set_brightness","{\"percent\":20}",true);break;
    case 2:
        configASSERT(navigation.brightness==0&&bsp_display_brightness()==20&&storage_ok);
        snprintf(args,sizeof(args),"{\"percent\":%u}",(original_brightness+1)*20);control_probe_call("self.display.set_brightness",args,true);break;
    case 3:
        configASSERT(navigation.brightness==original_brightness&&storage_ok);ESP_LOGI("control_probe","BRIGHTNESS_AND_FAILED_SEND_PASS");
        control_probe_call("self.apps.open","{\"app\":\"zen-muyu\"}",true);break;
    case 4:
        configASSERT(navigation.page==BADGE_PLAYING&&!strcmp(BADGE_GAMES_REGISTRY[navigation.game_selected].id,"zen-muyu"));ESP_LOGI("control_probe","MUYU_PASS");control_probe_start_xz();break;
    case 5:control_probe_call("self.apps.open","{\"app\":\"leo-radio\"}",true);break;
    case 6:
        configASSERT(navigation.page==BADGE_PLAYING&&!strcmp(BADGE_GAMES_REGISTRY[navigation.game_selected].id,"leo-radio"));ESP_LOGI("control_probe","RADIO_PASS");control_probe_start_xz();break;
    case 7:snprintf(args,sizeof(args),"{\"clip_id\":%u}",clip);control_probe_call("self.audio.play",args,true);break;
    case 8:
        configASSERT(navigation.page==BADGE_PLAYING&&!strcmp(BADGE_GAMES_REGISTRY[navigation.game_selected].id,"voice-keychain"));configASSERT(demo_voice_control_probe(clip));ESP_LOGI("control_probe","SPECIFIC_AUDIO_PASS");control_probe_start_xz();break;
    case 9:if(has_qr)control_probe_call("self.badge.show_qr","{}",true);break;
    case 10:if(has_qr){configASSERT(navigation.page==BADGE_QR);ESP_LOGI("control_probe","QR_PASS");}control_probe_start_xz();break;
    case 11:snprintf(args,sizeof(args),"{\"number\":%u}",target_badge+1);control_probe_call("self.badge.switch",args,true);break;
    case 12:
        configASSERT(navigation.page==BADGE_HOME&&navigation.active_badge==target_badge);configASSERT(profile_protocol_select(original_badge)==ESP_OK);ESP_LOGI("control_probe","BADGE_SWITCH_PASS");break;
    case 13:configASSERT(navigation.active_badge==original_badge);control_probe_start_xz();break;
    case 14:{
        uint32_t ticket=control_probe_call("self.apps.open","{\"app\":\"voice-keychain\"}",false);
        configASSERT(control_home());badge_control_commit(ticket,true);badge_control_command_t c;configASSERT(!badge_control_take(&c));break;
    }
    case 15:control_probe_start_xz();break;
    case 16:
        original_volume=demo_xiaozhi_probe_volume();control_probe_call("self.xiaozhi.set_volume","{\"percent\":60}",true);break;
    case 17:configASSERT(demo_xiaozhi_probe_volume()==60);control_probe_start_xz();break;
    case 18:
        configASSERT(demo_xiaozhi_probe_volume()==60);snprintf(args,sizeof(args),"{\"percent\":%u}",original_volume);control_probe_call("self.xiaozhi.set_volume",args,true);break;
    case 19:
        configASSERT(demo_xiaozhi_probe_volume()==original_volume);configASSERT(control_home());ESP_LOGI("control_probe","VOLUME_SAVE_REENTER_RESTORE_PASS");control_probe_start_xz();break;
    case 20:{
        badge_control_status_t state=badge_control_status();cJSON *a=cJSON_CreateObject();cJSON_AddStringToObject(a,"name",state.names[target_badge]);char *text=cJSON_PrintUnformatted(a);configASSERT(text);control_probe_call("self.badge.switch",text,true);cJSON_free(text);cJSON_Delete(a);break;
    }
    case 21:
        configASSERT(navigation.page==BADGE_HOME&&navigation.active_badge==target_badge);configASSERT(profile_protocol_select(original_badge)==ESP_OK);ESP_LOGI("control_probe","BADGE_BY_NAME_PASS");break;
    case 22:control_probe_start_xz();control_probe_call("self.reminder.set","{\"seconds\":8,\"text\":\"提醒测试\"}",true);break;
    case 23:configASSERT(reminder.deadline);control_probe_call("self.radio.search","{\"query\":\"中国之声\"}",true);control_probe_call("self.radio.play","{\"station_id\":0}",true);break;
    case 24:configASSERT(demo_radio_control_probe(0));ESP_LOGI("control_probe","SPECIFIED_RADIO_PASS");break;
    case 25:
        if(!reminder.ringing){stage--;break;}
        configASSERT(navigation.page==BADGE_HOME);ESP_LOGI("control_probe","REMINDER_AFTER_EXIT_RADIO_STOP_PASS");reminder_closing=true;break;
    case 26:
        if(reminder.ringing){stage--;break;}
        control_probe_start_xz();control_probe_call("self.reminder.set","{\"seconds\":8,\"text\":\"取消测试\"}",true);break;
    case 27:configASSERT(reminder.deadline);control_probe_call("self.reminder.cancel","{}",true);break;
    case 28:configASSERT(!reminder.deadline);ESP_LOGI("control_probe","REMINDER_CANCEL_PASS");control_probe_call("self.reminder.set","{\"seconds\":68,\"text\":\"息屏唤醒提醒\"}",true);break;
    case 29:
        saw_sleep|=badge_power_screen_off();
        if(!reminder.ringing){stage--;break;}
        configASSERT(saw_sleep&&!badge_power_screen_off()&&bsp_display_brightness()>0);ESP_LOGI("control_probe","REMINDER_SLEEP_WAKE_PASS");reminder_closing=true;break;
    case 30:if(reminder.ringing){stage--;break;}control_probe_start_xz();break;
    case 31:configASSERT(demo_xiaozhi_probe_volume()==original_volume);configASSERT(control_home());probe_radio_settings(true);break;
    case 32:configASSERT(!reminder.deadline&&!reminder.ringing);break;
    case 33:
        configASSERT(navigation.page==BADGE_HOME&&navigation.active_badge==original_badge&&navigation.brightness==original_brightness);
        control_probe_start_xz();break;
    case 34:
        homes_before=badge_ui_probe_home_count();control_probe_fail_prepare=true;
        control_probe_call("self.apps.open","{\"app\":\"voice-keychain\"}",true);break;
    case 35:
        configASSERT(navigation.page==BADGE_AI_CHAT&&demo_xiaozhi_active());
        configASSERT(badge_ui_probe_home_count()==homes_before);
        ESP_LOGI("control_probe","PREPARE_FAILURE_RESTORES_CHAT_PASS");
        control_probe_fail_start=true;control_probe_call("self.apps.open","{\"app\":\"voice-keychain\"}",true);break;
    case 36:
        configASSERT(navigation.page==BADGE_HOME&&!demo_xiaozhi_active());
        ESP_LOGI("control_probe","APP_FAILURE_RETURNS_HOME_PASS");control_probe_start_xz();break;
    case 37:control_probe_call("self.audio.play_random","{}",true);break;
    case 38:
        configASSERT(navigation.page==BADGE_PLAYING&&!strcmp(BADGE_GAMES_REGISTRY[navigation.game_selected].id,"voice-keychain"));
        ESP_LOGI("control_probe","RANDOM_AUDIO_DIRECT_PASS");control_probe_start_xz();break;
    case 39:control_probe_call("self.apps.open","{\"app\":\"voice-keychain\"}",true);break;
    case 40:
        configASSERT(navigation.page==BADGE_PLAYING&&!strcmp(BADGE_GAMES_REGISTRY[navigation.game_selected].id,"voice-keychain"));
        configASSERT(control_home());break;
    case 41:
        configASSERT(navigation.page==BADGE_HOME&&navigation.active_badge==original_badge&&navigation.brightness==original_brightness);
        configASSERT(xz_wake_enable(original_wake)==ESP_OK);
        ESP_LOGI("control_probe","ALL_PASS restored=1 heap=%lu stack=%u",(unsigned long)esp_get_free_heap_size(),(unsigned)uxTaskGetStackHighWaterMark(NULL));break;
    }
    stage++;
}
