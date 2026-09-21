#include "badge_alert.h"
#include "badge_power.h"
#include "bsp_audio.h"
#include "bsp_display.h"
#include "badge_theme.h"
#include "badge_footer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdatomic.h>
#include <math.h>
LV_FONT_DECLARE(font_xiaozhi_14);
static atomic_bool playing,cancelled;
static lv_obj_t *panel;
static void chime(void *arg){
    (void)arg;
    badge_power_enter();
    esp_err_t e=bsp_audio_init();
    unsigned previous=bsp_audio_get_volume();
    if(e==ESP_OK)e=bsp_audio_set_format(16000,16,1);
    if(e==ESP_OK)e=bsp_audio_wake();
    if(e==ESP_OK)bsp_audio_set_volume(90);
    int16_t pcm[320],silence[320]={0};
    for(unsigned i=0;i<320;i++)pcm[i]=(int16_t)(5000*sinf(6.2831853f*800*i/16000));
    for(unsigned frame=0;e==ESP_OK&&frame<90&&!atomic_load(&cancelled);frame++){
        e=bsp_audio_write(frame%30<10?pcm:silence,sizeof(pcm));
    }
    bsp_audio_set_volume(previous);
    esp_err_t sleep=badge_power_leave();
    ESP_LOGI("badge_alert","Chime done audio=%s sleep=%s stack=%u",esp_err_to_name(e),esp_err_to_name(sleep),(unsigned)uxTaskGetStackHighWaterMark(NULL));
    atomic_store(&playing,false);vTaskDelete(NULL);
}
bool badge_alert_start(const char *text){
    if(panel||atomic_load(&playing)||!bsp_lvgl_lock(1000))return false;
    const uint32_t *c=badge_theme_colors();
    panel=lv_obj_create(lv_layer_top());lv_obj_remove_style_all(panel);
    lv_obj_set_pos(panel,0,28);lv_obj_set_size(panel,240,292);
    lv_obj_set_style_bg_color(panel,lv_color_hex(c[BADGE_BACKGROUND]),0);lv_obj_set_style_bg_opa(panel,LV_OPA_COVER,0);
    lv_obj_remove_flag(panel,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *title=lv_label_create(panel);lv_obj_set_pos(title,20,46);lv_obj_set_width(title,200);
    lv_obj_set_style_text_font(title,&font_xiaozhi_14,0);lv_obj_set_style_text_color(title,lv_color_hex(c[BADGE_ACCENT]),0);lv_label_set_text(title,"提醒时间到了");
    lv_obj_t *body=lv_label_create(panel);lv_obj_set_pos(body,20,90);lv_obj_set_size(body,200,140);
    lv_obj_set_style_text_font(body,&font_xiaozhi_14,0);lv_obj_set_style_text_line_space(body,8,0);lv_obj_set_style_text_color(body,lv_color_hex(c[BADGE_TEXT]),0);
    lv_label_set_long_mode(body,LV_LABEL_LONG_WRAP);lv_label_set_text(body,text);
    lv_obj_t *hint=badge_footer_create(panel,BADGE_HINT_REMINDER);lv_obj_set_y(hint,272);
    bsp_lvgl_unlock();atomic_store(&cancelled,false);atomic_store(&playing,true);
    if(xTaskCreate(chime,"badge_chime",6144,NULL,4,NULL)!=pdPASS){atomic_store(&playing,false);ESP_LOGW("badge_alert","Chime task unavailable; visual reminder remains");}
    return true;
}
bool badge_alert_dismiss(void){
    atomic_store(&cancelled,true);if(atomic_load(&playing))return false;
    if(!bsp_lvgl_lock(1000))return false;
    if(panel){lv_obj_delete(panel);panel=NULL;}bsp_lvgl_unlock();return true;
}
