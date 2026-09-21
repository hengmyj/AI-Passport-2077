#include "shengbei_app.h"
#include "badge_theme.h"
#include "badge_header.h"
#include "badge_footer.h"
#include "bsp_display.h"
#include "esp_random.h"
#include "lvgl.h"
#include <stdio.h>

LV_FONT_DECLARE(font_muyu_14);
LV_FONT_DECLARE(font_badge_28);

typedef enum { CUP_NONE, CUP_SHENG, CUP_XIAO, CUP_YIN } cup_result_t;
static lv_obj_t *screen,*title,*subtitle,*left_cup,*right_cup,*left_note,*right_note,*result,*advice,*rule,*count_label,*footer,*result_tag;
static lv_obj_t *labels[14];
static unsigned roles[14],label_count,cast_count;
static bool left_up,right_up;
static cup_result_t current;
static uint32_t theme_seen;

static lv_obj_t *label(const char *text,int x,int y,int w,const lv_font_t *font,unsigned role){
    lv_obj_t *o=lv_label_create(screen);lv_obj_set_pos(o,x,y);lv_obj_set_size(o,w,30);
    lv_obj_set_style_text_font(o,font,0);lv_obj_set_style_text_color(o,lv_color_hex(badge_theme_colors()[role]),0);
    lv_label_set_long_mode(o,LV_LABEL_LONG_WRAP);lv_label_set_text(o,text);
    if(label_count<14){labels[label_count]=o;roles[label_count++]=role;}
    return o;
}
static void track(lv_obj_t *o,unsigned role){
    if(label_count<14){labels[label_count]=o;roles[label_count++]=role;}
}
static void rect(lv_layer_t *l,int x,int y,int w,int h,int radius,uint32_t fill,uint32_t border,int bw){
    lv_draw_rect_dsc_t d;lv_draw_rect_dsc_init(&d);d.bg_color=lv_color_hex(fill);d.bg_opa=LV_OPA_COVER;d.border_color=lv_color_hex(border);d.border_width=bw;d.radius=radius;
    lv_area_t a={x,y,x+w-1,y+h-1};lv_draw_rect(l,&d,&a);
}
static void cup_face(lv_layer_t *l,int x,int y,bool known,bool up){
    const uint32_t *c=badge_theme_colors();
    if(!known){
        rect(l,x,y,76,74,14,c[BADGE_BACKGROUND],c[BADGE_MUTED],2);
        rect(l,x+21,y+25,34,24,12,c[BADGE_PANEL],c[BADGE_MUTED],1);
        return;
    }
    if(up){
        /* 正面：亮色实心面。 */
        rect(l,x,y,76,74,14,c[BADGE_ACCENT],c[BADGE_TEXT],1);
    }else{
        /* 反面：深色空心杯口。 */
        rect(l,x,y,76,74,14,c[BADGE_BACKGROUND],c[BADGE_ACCENT],3);
        rect(l,x+16,y+18,44,32,14,c[BADGE_PANEL],c[BADGE_MUTED],1);
    }
}
static void cup_art(lv_event_t *event){
    lv_layer_t *l=lv_event_get_layer(event);const uint32_t *c=badge_theme_colors();
    rect(l,14,62,212,204,8,c[BADGE_BACKGROUND],c[BADGE_PANEL],1);
    cup_face(l,31,104,current!=CUP_NONE,left_up);
    cup_face(l,133,104,current!=CUP_NONE,right_up);
}
static void apply_theme(void){
    uint32_t r=badge_theme_revision();if(theme_seen==r)return;const uint32_t *c=badge_theme_colors();
    lv_obj_set_style_bg_color(screen,lv_color_hex(c[BADGE_BACKGROUND]),0);
    for(unsigned i=0;i<label_count;i++)lv_obj_set_style_text_color(labels[i],lv_color_hex(c[roles[i]]),0);
    lv_obj_set_style_bg_color(left_cup,lv_color_hex(c[BADGE_PANEL]),0);lv_obj_set_style_border_color(left_cup,lv_color_hex(c[BADGE_ACCENT]),0);
    lv_obj_set_style_bg_color(right_cup,lv_color_hex(c[BADGE_PANEL]),0);lv_obj_set_style_border_color(right_cup,lv_color_hex(c[BADGE_ACCENT]),0);
    lv_obj_set_style_text_color(footer,lv_color_hex(c[BADGE_MUTED]),0);
    theme_seen=r;lv_obj_invalidate(screen);
}
static const char *name_for(cup_result_t r){
    return r==CUP_SHENG?"圣杯":r==CUP_XIAO?"笑杯":r==CUP_YIN?"阴杯":"未投掷";
}
static const char *advice_for(cup_result_t r){
    return r==CUP_SHENG?"一正一反：可以推进。":r==CUP_XIAO?"两正：先别急，再确认一次。":r==CUP_YIN?"两反：暂缓，换个方向。":"默念问题后按 OK。";
}
static void refresh(void){
    char n[16];snprintf(n,sizeof(n),"%u次",cast_count);lv_label_set_text(count_label,n);
    lv_label_set_text(result,name_for(current));lv_label_set_text(advice,advice_for(current));
    lv_label_set_text(left_cup,current==CUP_NONE?"？":left_up?"正":"反");
    lv_label_set_text(right_cup,current==CUP_NONE?"？":right_up?"正":"反");
    lv_label_set_text(left_note,current==CUP_NONE?"待掷":left_up?"正面":"反面");
    lv_label_set_text(right_note,current==CUP_NONE?"待掷":right_up?"正面":"反面");
    uint32_t accent=badge_theme_colors()[BADGE_ACCENT],muted=badge_theme_colors()[BADGE_MUTED];
    lv_obj_set_style_text_color(result,lv_color_hex(current==CUP_NONE?muted:accent),0);
    if(result_tag)lv_label_set_text(result_tag,current==CUP_NONE?"等待":current==CUP_SHENG?"可行":current==CUP_XIAO?"再问":"暂缓");
    apply_theme();lv_obj_invalidate(screen);
}
static void cast(void){
    uint32_t v=esp_random();left_up=v&1;right_up=v&2;cast_count++;
    current=(left_up!=right_up)?CUP_SHENG:(left_up?CUP_XIAO:CUP_YIN);
    refresh();
}
void demo_shengbei_enter(void){
    screen=lv_obj_create(NULL);lv_obj_remove_flag(screen,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(screen,LV_OPA_COVER,0);lv_obj_add_event_cb(screen,cup_art,LV_EVENT_DRAW_MAIN,NULL);lv_screen_load(screen);
    badge_header_attach(screen);
    title=label("圣杯决策",14,40,120,&font_muyu_14,BADGE_TEXT);
    subtitle=label("默念问题",132,42,92,&font_muyu_14,BADGE_ACCENT);
    rule=label("一正一反为圣杯",22,72,196,&font_muyu_14,BADGE_MUTED);
    left_cup=lv_label_create(screen);lv_obj_set_pos(left_cup,45,119);lv_obj_set_size(left_cup,48,36);
    right_cup=lv_label_create(screen);lv_obj_set_pos(right_cup,147,119);lv_obj_set_size(right_cup,48,36);
    for(lv_obj_t *o=left_cup;o;o=(o==left_cup?right_cup:NULL)){
        lv_obj_set_style_text_font(o,&font_badge_28,0);lv_obj_set_style_text_align(o,LV_TEXT_ALIGN_CENTER,0);
        lv_obj_set_style_pad_top(o,2,0);lv_obj_set_style_radius(o,12,0);lv_obj_set_style_border_width(o,0,0);lv_obj_set_style_bg_opa(o,LV_OPA_TRANSP,0);
        lv_obj_set_style_text_color(o,lv_color_hex(badge_theme_colors()[BADGE_TEXT]),0);track(o,BADGE_TEXT);
    }
    left_note=label("待掷",44,178,52,&font_muyu_14,BADGE_MUTED);
    right_note=label("待掷",146,178,52,&font_muyu_14,BADGE_MUTED);
    result_tag=label("等待",22,194,196,&font_muyu_14,BADGE_MUTED);
    lv_obj_set_style_text_align(result_tag,LV_TEXT_ALIGN_CENTER,0);
    result=label("未投掷",22,210,196,&font_badge_28,BADGE_ACCENT);
    lv_obj_set_style_text_align(result,LV_TEXT_ALIGN_CENTER,0);
    advice=label("默念问题后按 OK。",24,246,192,&font_muyu_14,BADGE_MUTED);lv_obj_set_height(advice,36);
    lv_obj_set_style_text_align(advice,LV_TEXT_ALIGN_CENTER,0);
    count_label=label("0次",182,72,38,&font_muyu_14,BADGE_MUTED);
    lv_obj_set_style_text_align(count_label,LV_TEXT_ALIGN_RIGHT,0);
    lv_label_set_long_mode(count_label,LV_LABEL_LONG_CLIP);
    footer=badge_footer_create(screen,BADGE_HINT_SHENGBEI);
    current=CUP_NONE;left_up=false;right_up=false;theme_seen=0;refresh();
}
void demo_shengbei_exit(void){if(screen)lv_obj_delete(screen);screen=NULL;label_count=0;theme_seen=0;}
esp_err_t demo_shengbei_start(void){return ESP_OK;}
esp_err_t demo_shengbei_stop(void){return ESP_OK;}
void demo_shengbei_key(bsp_btn_t button,bsp_btn_ev_t event){
    if(button!=BSP_BTN_OK||(event!=BSP_BTN_CLICK&&event!=BSP_BTN_DOUBLE))return;
    if(bsp_lvgl_lock(1000)){cast();bsp_lvgl_unlock();}
}





