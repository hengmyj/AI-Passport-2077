#include "radio_ui.h"
#include "radio_logic.h"
extern "C" {
#include "badge_theme.h"
#include "badge_header.h"
#include "badge_footer.h"
#include "lvgl.h"
LV_FONT_DECLARE(font_radio_14);
LV_FONT_DECLARE(font_badge_10);
LV_FONT_DECLARE(font_badge_28);
}
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cstdarg>
static void label_text(lv_obj_t *o,const char *value){if(strcmp(lv_label_get_text(o),value))lv_label_set_text(o,value);}
static void label_text_fmt(lv_obj_t *o,const char *format,...){char value[128];va_list args;va_start(args,format);vsnprintf(value,sizeof(value),format,args);va_end(args);label_text(o,value);}
static void background(lv_obj_t *o,lv_color_t color,lv_style_selector_t selector){if(!lv_color_eq(lv_obj_get_style_bg_color(o,LV_PART_MAIN),color))lv_obj_set_style_bg_color(o,color,selector);}
static void border_color(lv_obj_t *o,lv_color_t color,lv_style_selector_t selector){if(!lv_color_eq(lv_obj_get_style_border_color(o,LV_PART_MAIN),color))lv_obj_set_style_border_color(o,color,selector);}
static void border_width(lv_obj_t *o,int value,lv_style_selector_t selector){if(lv_obj_get_style_border_width(o,LV_PART_MAIN)!=value)lv_obj_set_style_border_width(o,value,selector);}
static lv_obj_t *screen,*title,*city,*counter,*panel,*station,*frequency,*mode,*needle,*status,*hint;
static bool draw_main=false;static uint8_t audio_levels[18];
static lv_obj_t *tick_labels[6],*rows[6],*row_names[6],*volume_text;
static radio_page_t previous_page=RADIO_TIMER;
static uint16_t previous_frequency=0;static unsigned previous_count=0;
static unsigned previous_index=~0u;static uint32_t previous_theme;
static char previous_station[64];
static bool rendered;
static radio_controls_t last_controls;
static radio_ui_snapshot_t last_state;
static unsigned last_left;
static bool same_metadata(const radio_controls_t &c,const radio_ui_snapshot_t &s,unsigned left){
    return rendered&&previous_theme==badge_theme_revision()&&last_left==left&&
        c.page==last_controls.page&&c.selected==last_controls.selected&&c.volume==last_controls.volume&&
        c.city==last_controls.city&&c.timer==last_controls.timer&&s.index==last_state.index&&s.count==last_state.count&&
        s.volume==last_state.volume&&s.state==last_state.state&&s.station.frequency_decihz==last_state.station.frequency_decihz&&
        !strcmp(s.station.name,last_state.station.name)&&!strcmp(s.location,last_state.location)&&!strcmp(s.detail,last_state.detail);
}
static lv_obj_t *box(lv_obj_t *parent,int x,int y,int w,int h){auto *o=lv_obj_create(parent);lv_obj_remove_style_all(o);lv_obj_set_pos(o,x,y);lv_obj_set_size(o,w,h);lv_obj_set_style_bg_opa(o,LV_OPA_COVER,0);lv_obj_remove_flag(o,LV_OBJ_FLAG_SCROLLABLE);return o;}
static lv_obj_t *label(lv_obj_t *parent,const char *text,int x,int y,int w,const lv_font_t *font){auto *o=lv_label_create(parent);lv_obj_set_pos(o,x,y);lv_obj_set_width(o,w);lv_obj_set_style_text_font(o,font,0);lv_obj_set_style_text_letter_space(o,0,0);lv_label_set_long_mode(o,LV_LABEL_LONG_CLIP);label_text(o,text);return o;}
static void visible(lv_obj_t *o,bool yes){if(yes)lv_obj_remove_flag(o,LV_OBJ_FLAG_HIDDEN);else lv_obj_add_flag(o,LV_OBJ_FLAG_HIDDEN);}
static void ink(lv_obj_t *o,unsigned color){lv_color_t value=lv_color_hex(badge_theme_colors()[color]);if(!lv_color_eq(lv_obj_get_style_text_color(o,LV_PART_MAIN),value))lv_obj_set_style_text_color(o,value,0);}
static void needle_x(void *o,int32_t value){lv_obj_set_x(static_cast<lv_obj_t *>(o),value);}
static void artwork(lv_event_t *event){
    if(!draw_main)return;
    auto *layer=lv_event_get_layer(event);const auto *c=badge_theme_colors();
    for(unsigned i=0;i<41;i++){lv_draw_line_dsc_t d;lv_draw_line_dsc_init(&d);d.p1={static_cast<lv_value_precise_t>(16+i*5),177};d.p2={d.p1.x,static_cast<lv_value_precise_t>(177+(i%8==0?16:i%4==0?11:6))};d.width=1;d.color=lv_color_hex(c[BADGE_MUTED]);d.opa=LV_OPA_COVER;lv_draw_line(layer,&d);}
    for(unsigned i=0;i<18;i++){int h=std::max(1,std::min<int>(audio_levels[i],100)*26/100);lv_draw_rect_dsc_t d;lv_draw_rect_dsc_init(&d);d.bg_color=lv_color_hex(c[BADGE_ACCENT]);d.bg_opa=LV_OPA_COVER;lv_area_t a={static_cast<int>(18+i*11),248-h,static_cast<int>(23+i*11),247};lv_draw_rect(layer,&d,&a);}
}
void radio_ui_create(){
    screen=box(nullptr,0,0,240,320);draw_main=false;rendered=false;
    lv_obj_add_event_cb(screen,artwork,LV_EVENT_DRAW_MAIN,nullptr);badge_header_attach(screen);
    title=label(screen,"CITY RADIO",14,34,214,&font_badge_28);city=label(screen,"城市网络电台",14,65,154,&font_radio_14);counter=label(screen,"",169,68,57,&font_badge_10);lv_obj_set_style_text_align(counter,LV_TEXT_ALIGN_RIGHT,0);
    panel=box(screen,14,92,212,68);station=label(panel,"等待电台目录",12,10,188,&font_radio_14);lv_label_set_long_mode(station,LV_LABEL_LONG_SCROLL_CIRCULAR);
    frequency=label(panel,"NET",12,31,120,&font_badge_28);mode=label(panel,"LIVE STREAM",140,47,66,&font_badge_10);
    for(unsigned i=0;i<6;i++){char text[8];snprintf(text,sizeof(text),"%u",88+i*4);tick_labels[i]=label(screen,text,10+i*40,201,30,&font_badge_10);}
    needle=box(screen,115,169,2,27);
    for(unsigned i=0;i<6;i++){rows[i]=box(screen,14,96+i*25,212,24);row_names[i]=label(rows[i],"",10,3,192,&font_radio_14);}
    volume_text=label(screen,"40%",30,145,180,&font_badge_28);lv_obj_set_style_text_align(volume_text,LV_TEXT_ALIGN_CENTER,0);
    status=label(screen,"",14,259,212,&font_radio_14);hint=badge_footer_create(screen,"");
    previous_index=~0u;previous_station[0]=0;previous_theme=0;previous_page=RADIO_TIMER;lv_screen_load(screen);
}
void radio_ui_render(const radio_controls_t &c,const radio_ui_snapshot_t &s,unsigned left){
    if(!screen)return;
    badge_header_battery(s.battery);
    const auto *colors=badge_theme_colors();const bool main=c.page==RADIO_MAIN;
    bool levels_changed=false;
    for(unsigned i=0;i<18;i++){uint8_t level=s.state==RadioPlaybackState::Playing?s.levels[i]:0;if(audio_levels[i]!=level){audio_levels[i]=level;levels_changed=true;}}
    if(main&&levels_changed){lv_area_t area={14,220,225,249};lv_obj_invalidate_area(screen,&area);}
    if(same_metadata(c,s,left))return;
    last_controls=c;last_state=s;last_left=left;rendered=true;
    const bool changed=previous_page!=c.page;previous_page=c.page;
    background(screen,lv_color_hex(colors[BADGE_BACKGROUND]),0);background(panel,lv_color_hex(colors[BADGE_PANEL]),0);
    ink(title,BADGE_ACCENT);ink(city,BADGE_TEXT);ink(counter,BADGE_MUTED);ink(station,BADGE_TEXT);ink(frequency,BADGE_ACCENT);ink(mode,BADGE_MUTED);ink(status,BADGE_ACCENT);ink(hint,BADGE_MUTED);ink(volume_text,BADGE_TEXT);
    const char *head=c.page==RADIO_SETTINGS?"电台设置":c.page==RADIO_VOLUME?"音量":c.page==RADIO_CITY?"选择城市":c.page==RADIO_TIMER?"定时停止":s.location;
    label_text(city,head);if(main&&s.count)label_text_fmt(counter,"%u / %u",s.index+1,s.count);else label_text(counter,"");
    visible(panel,main);visible(needle,main);visible(volume_text,c.page==RADIO_VOLUME);
    for(unsigned i=0;i<6;i++){visible(tick_labels[i],main);ink(tick_labels[i],BADGE_MUTED);}
    draw_main=main;if(changed){lv_area_t area={10,169,225,249};lv_obj_invalidate_area(screen,&area);}
    background(needle,lv_color_hex(colors[BADGE_ACCENT]),0);
    if(main){
        const char *name=s.station.name[0]?s.station.name:"等待电台目录";
        if(strcmp(previous_station,name)||previous_index!=s.index){label_text(station,name);snprintf(previous_station,sizeof(previous_station),"%s",name);}
        if(s.station.frequency_decihz)label_text_fmt(frequency,"%u.%u",s.station.frequency_decihz/10,s.station.frequency_decihz%10);else label_text(frequency,"NET");
        if(changed||previous_index!=s.index||previous_theme!=badge_theme_revision()||previous_frequency!=s.station.frequency_decihz||previous_count!=s.count){
            int x=16+radio_dial_position(s.station.frequency_decihz,s.index,s.count)*200/1000;
            lv_anim_delete(needle,needle_x);lv_anim_t a;lv_anim_init(&a);lv_anim_set_var(&a,needle);lv_anim_set_exec_cb(&a,needle_x);lv_anim_set_values(&a,lv_obj_get_x(needle),x);lv_anim_set_duration(&a,260);lv_anim_set_path_cb(&a,lv_anim_path_ease_out);lv_anim_start(&a);
        }
        previous_index=s.index;previous_frequency=s.station.frequency_decihz;previous_count=s.count;
    }
    previous_theme=badge_theme_revision();
    static const char *settings[]={"音量","城市电台","定时停止"};
    unsigned count=c.page==RADIO_SETTINGS?3:c.page==RADIO_CITY?8:c.page==RADIO_TIMER?5:0;
    unsigned first=c.selected<6?0:c.selected-5;
    for(unsigned i=0;i<6;i++){
        unsigned item=first+i;visible(rows[i],item<count);if(item>=count)continue;
        bool chosen=item==c.selected;background(rows[i],lv_color_hex(colors[chosen?BADGE_PANEL:BADGE_BACKGROUND]),0);border_width(rows[i],chosen?1:0,0);border_color(rows[i],lv_color_hex(colors[BADGE_ACCENT]),0);ink(row_names[i],chosen?BADGE_TEXT:BADGE_MUTED);
        char text[80]={};
        if(c.page==RADIO_SETTINGS)snprintf(text,sizeof(text),"%s",settings[item]);
        if(c.page==RADIO_CITY)snprintf(text,sizeof(text),"%s",RADIO_CITIES[item]);
        if(c.page==RADIO_TIMER){if(item)snprintf(text,sizeof(text),"%u 分钟",RADIO_TIMER_MINUTES[item]);else snprintf(text,sizeof(text),"关闭定时");}
        label_text(row_names[i],text);
    }
    label_text_fmt(volume_text,"%d%%",c.volume);
    if(left)label_text_fmt(status,"音量 %d%%  %02u:%02u",s.volume,left/60,left%60);
    else if(s.state==RadioPlaybackState::Playing)label_text_fmt(status,"播放中  音量 %d%%",s.volume);
    else label_text(status,s.detail);
    label_text(hint,main?BADGE_HINT_RADIO:c.page==RADIO_VOLUME?BADGE_HINT_VOLUME:BADGE_HINT_SELECT);
}
void radio_ui_destroy(){if(screen){lv_anim_delete(needle,needle_x);lv_obj_delete(screen);screen=nullptr;}}
