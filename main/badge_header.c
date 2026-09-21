#include "badge_header.h"
#include "badge_theme.h"
#include "badge_clock.h"
#include <time.h>
LV_FONT_DECLARE(font_badge_10);
static lv_obj_t *owner,*brand,*power,*slot,*clock_label;
static unsigned badge_active,badge_count;
static int battery=-1,shown_battery=-2;
static bool connected;
static uint32_t theme_seen;
static void artwork(lv_event_t *e) {
    if(lv_event_get_code(e)==LV_EVENT_DELETE){
        if(lv_event_get_target(e)==owner)owner=brand=power=slot=clock_label=NULL;
        return;
    }
    lv_layer_t *l=lv_event_get_layer(e);const uint32_t *c=badge_theme_colors();
    lv_draw_rect_dsc_t r;lv_draw_rect_dsc_init(&r);r.bg_opa=LV_OPA_COVER;
    r.bg_color=lv_color_hex(((c[BADGE_ACCENT]&0xfefefe)>>1)+((c[BADGE_PANEL]&0xfefefe)>>1));
    lv_area_t line={14,28,225,28};lv_draw_rect(l,&r,&line);
    if(!connected)return;
    lv_draw_arc_dsc_t a;lv_draw_arc_dsc_init(&a);a.center=(lv_point_t){186,22};
    a.width=1;a.start_angle=225;a.end_angle=315;a.color=lv_color_hex(c[BADGE_TEXT]);a.opa=LV_OPA_COVER;
    a.radius=7;lv_draw_arc(l,&a);a.radius=4;lv_draw_arc(l,&a);
    r.bg_color=a.color;lv_area_t dot={186,22,186,22};lv_draw_rect(l,&r,&dot);
}
void badge_header_refresh(void) {
    if(!owner)return;
    char clock_text[6];badge_clock_format((int64_t)time(NULL),clock_text);
    if(strcmp(lv_label_get_text(clock_label),clock_text))lv_label_set_text(clock_label,clock_text);
    if(theme_seen!=badge_theme_revision()){
        const uint32_t *c=badge_theme_colors();
        lv_obj_set_style_text_color(brand,lv_color_hex(c[BADGE_MUTED]),0);
        lv_obj_set_style_text_color(power,lv_color_hex(c[BADGE_TEXT]),0);
        lv_obj_set_style_text_color(clock_label,lv_color_hex(c[BADGE_TEXT]),0);
        lv_obj_set_style_text_color(slot,lv_color_hex(c[BADGE_MUTED]),0);
        theme_seen=badge_theme_revision();lv_area_t area={0,0,239,28};lv_obj_invalidate_area(owner,&area);
    }
    if(shown_battery!=battery){
        if(battery<0)lv_label_set_text(power,"--");else lv_label_set_text_fmt(power,"%d%%",battery);
        shown_battery=battery;
    }
}
void badge_header_attach(lv_obj_t *screen) {
    owner=screen;while(lv_obj_remove_event_cb(screen,artwork)){}
    lv_obj_add_event_cb(screen,artwork,LV_EVENT_DRAW_MAIN,NULL);
    lv_obj_add_event_cb(screen,artwork,LV_EVENT_DELETE,NULL);
    brand=lv_label_create(screen);power=lv_label_create(screen);slot=lv_label_create(screen);clock_label=lv_label_create(screen);
    lv_obj_t *labels[]={brand,power,slot,clock_label};
    for(unsigned i=0;i<4;i++){
        lv_obj_set_style_text_font(labels[i],&font_badge_10,0);
        lv_obj_set_style_text_letter_space(labels[i],0,0);
        lv_label_set_long_mode(labels[i],LV_LABEL_LONG_CLIP);
    }
    lv_obj_set_pos(brand,14,12);lv_obj_set_width(brand,78);lv_label_set_text(brand,"AI Passport");
    lv_obj_set_pos(clock_label,93,12);lv_obj_set_width(clock_label,54);lv_obj_set_style_text_align(clock_label,LV_TEXT_ALIGN_CENTER,0);
    lv_obj_set_pos(slot,148,12);lv_obj_set_width(slot,26);lv_obj_set_style_text_align(slot,LV_TEXT_ALIGN_RIGHT,0);
    if(badge_count)lv_label_set_text_fmt(slot,"%u/%u",badge_active+1,badge_count);else lv_label_set_text(slot,"");
    lv_obj_set_pos(power,194,12);lv_obj_set_width(power,32);lv_obj_set_style_text_align(power,LV_TEXT_ALIGN_RIGHT,0);
    theme_seen=0;shown_battery=-2;badge_header_refresh();
}
void badge_header_badge(unsigned active,unsigned count){
    if(badge_active==active&&badge_count==count)return;
    badge_active=active;badge_count=count;
    if(slot){if(count)lv_label_set_text_fmt(slot,"%u/%u",active+1,count);else lv_label_set_text(slot,"");}
}
void badge_header_battery(int percent){battery=percent<0?-1:percent>100?100:percent;badge_header_refresh();}
void badge_header_network(bool value){
    if(connected==value)return;
    connected=value;if(owner){lv_area_t area={179,14,193,23};lv_obj_invalidate_area(owner,&area);}
}
