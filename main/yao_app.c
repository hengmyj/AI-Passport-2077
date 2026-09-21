#include "yao_app.h"
#include "yao_ui.h"
#include "badge_network.h"
#include "bsp_display.h"
#include <stdio.h>
#include <string.h>
static uint8_t lines[6];
static unsigned count,last_coins;
static yao_record_t current;
static bool interpretation;
static char location_state[96];
static void read_location_state(char out[96]){
    yao_location_t p=yao_location_get();snprintf(out,96,"%s|%ld|%s",yao_location_status(),(long)p.longitude_e6,p.region);
}
static bool refresh_location(void){
    if(count!=6||!yao_refresh_location(&current))return false;
    yao_ui_set_time(current.cast_utc,&current.location);return true;
}
void demo_yao_enter(void){
    interpretation=false;
    if(!count||count==6){yao_record_t last;if(yao_latest(&last)){current=last;memcpy(lines,last.lines,6);count=6;}}
    refresh_location();yao_ui_create();yao_ui_set_time(current.cast_utc,&current.location);yao_ui_render(lines,count,last_coins,current.id,NULL);read_location_state(location_state);
}
void demo_yao_exit(void){yao_ui_destroy();}
esp_err_t demo_yao_start(void){yao_location_recheck();yao_location_poll();return badge_network_prepare_entropy();}
esp_err_t demo_yao_stop(void){interpretation=false;return ESP_OK;}
void demo_yao_notice(const char *notice){if(bsp_lvgl_lock(1000)){yao_ui_render(lines,count,last_coins,current.id,notice);bsp_lvgl_unlock();}}
void demo_yao_key(bsp_btn_t button,bsp_btn_ev_t event){
    if(event!=BSP_BTN_CLICK&&event!=BSP_BTN_LONG)return;
    if(button==BSP_BTN_OK&&event==BSP_BTN_CLICK&&count==6){interpretation=true;return;}
    if(!bsp_lvgl_lock(1000))return;
    if(button==BSP_BTN_DOWN&&event==BSP_BTN_LONG){count=0;memset(lines,0,6);current=(yao_record_t){0};yao_ui_render(lines,count,last_coins,0,NULL);}
    else if(event==BSP_BTN_CLICK){
        if(button==BSP_BTN_OK&&count<6){
            uint32_t bits;
            if(!yao_random_bits(&bits)){yao_ui_render(lines,count,last_coins,0,"随机源准备中，请稍后再按 OK");}
            else {
                last_coins=bits&7u;lines[count++]=yao_coin_line(bits);
                if(count==6&&!yao_save_manual(lines,&current)){count--;yao_ui_render(lines,count,last_coins,0,"结果保存失败，请重试");}
                else {yao_ui_set_time(current.cast_utc,&current.location);yao_ui_render(lines,count,last_coins,current.id,NULL);}
            }
        }else if(count==6)yao_ui_scroll(button==BSP_BTN_UP?-1:1);
    }
    bsp_lvgl_unlock();
}
void demo_yao_tick(void){
    char next[96];read_location_state(next);bool changed=strcmp(next,location_state)!=0;
    if(refresh_location())changed=true;
    if(changed&&bsp_lvgl_lock(100)){yao_ui_render(lines,count,last_coins,current.id,NULL);bsp_lvgl_unlock();read_location_state(location_state);}
}
bool demo_yao_take_interpretation(yao_record_t *record){if(!interpretation||count!=6)return false;interpretation=false;refresh_location();*record=current;return true;}
