#include "xiaozhi_style.h"
#include "nvs.h"
#include "esp_log.h"
#include <stdatomic.h>
static atomic_uint current_style;
void xiaozhi_style_init(void){
    uint8_t value=0;nvs_handle_t n;
    if(nvs_open("xiaozhi",NVS_READONLY,&n)==ESP_OK){if(nvs_get_u8(n,"face_style",&value)!=ESP_OK)value=0;nvs_close(n);}
    atomic_store(&current_style,value<XZ_STYLE_COUNT?value:XZ_STYLE_CUTE);
}
unsigned xiaozhi_style_get(void){return atomic_load(&current_style);}
bool xiaozhi_style_set(unsigned style){
    if(style>=XZ_STYLE_COUNT)return false;
    nvs_handle_t n;esp_err_t e=nvs_open("xiaozhi",NVS_READWRITE,&n);
    if(e==ESP_OK){e=nvs_set_u8(n,"face_style",(uint8_t)style);if(e==ESP_OK)e=nvs_commit(n);nvs_close(n);}
    if(e!=ESP_OK){ESP_LOGW("xz_style","Style save failed: %d",(int)e);return false;}
    atomic_store(&current_style,style);return true;
}
