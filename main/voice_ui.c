#include "voice_ui.h"
#include "badge_theme.h"
#include "badge_header.h"
#include "badge_footer.h"
#include "lvgl.h"
#include <stdio.h>
LV_FONT_DECLARE(font_voice_14);
LV_FONT_DECLARE(font_badge_10);
LV_FONT_DECLARE(font_badge_28);
static lv_obj_t *screen,*title,*subtitle,*counter,*rows[6],*names[6],*numbers[6],*status,*hint,*footer;
static lv_obj_t *volume_text,*volume_bar;
static unsigned seen_pack,seen_clip;
static int seen_view,seen_playing,seen_volume,seen_battery;
static uint32_t seen_theme;
static const char *seen_error;
static lv_obj_t *label(lv_obj_t *parent,const char *text,int x,int y,int width,const lv_font_t *font) {
    lv_obj_t *o=lv_label_create(parent);lv_obj_set_pos(o,x,y);lv_obj_set_width(o,width);
    lv_obj_set_style_text_font(o,font,0);lv_obj_set_style_text_letter_space(o,0,0);
    lv_label_set_long_mode(o,LV_LABEL_LONG_CLIP);lv_label_set_text(o,text);return o;
}
static void color(lv_obj_t *o,unsigned role){lv_obj_set_style_text_color(o,lv_color_hex(badge_theme_colors()[role]),0);}
void voice_ui_create(void) {
    screen=lv_obj_create(NULL);lv_obj_remove_style_all(screen);lv_obj_set_size(screen,240,320);
    lv_obj_set_style_bg_opa(screen,LV_OPA_COVER,0);lv_obj_remove_flag(screen,LV_OBJ_FLAG_SCROLLABLE);
    badge_header_attach(screen);
    title=label(screen,"VOICE",14,34,180,&font_badge_28);
    counter=label(screen,"",154,66,72,&font_badge_10);lv_obj_set_style_text_align(counter,LV_TEXT_ALIGN_RIGHT,0);
    subtitle=label(screen,"",14,65,136,&font_voice_14);
    for(unsigned i=0;i<6;i++) {
        rows[i]=lv_obj_create(screen);lv_obj_remove_style_all(rows[i]);lv_obj_set_pos(rows[i],14,91+i*25);lv_obj_set_size(rows[i],212,24);
        lv_obj_remove_flag(rows[i],LV_OBJ_FLAG_SCROLLABLE);lv_obj_set_style_bg_opa(rows[i],LV_OPA_COVER,0);
        numbers[i]=label(rows[i],"",5,5,22,&font_badge_10);
        names[i]=label(rows[i],"",31,2,174,&font_voice_14);
        lv_obj_set_height(names[i],18);
    }
    volume_text=label(screen,"",30,119,180,&font_badge_28);lv_obj_set_style_text_align(volume_text,LV_TEXT_ALIGN_CENTER,0);
    volume_bar=lv_bar_create(screen);lv_obj_set_pos(volume_bar,30,179);lv_obj_set_size(volume_bar,180,8);lv_bar_set_range(volume_bar,0,100);
    footer=lv_obj_create(screen);lv_obj_remove_style_all(footer);lv_obj_set_pos(footer,0,250);lv_obj_set_size(footer,240,70);lv_obj_set_style_bg_opa(footer,LV_OPA_COVER,0);
    status=label(screen,"",14,255,212,&font_voice_14);
    hint=badge_footer_create(screen,"");
    seen_view=-1;seen_playing=-2;seen_volume=-1;seen_battery=-2;seen_theme=0;seen_error=(void *)1;
    lv_screen_load(screen);
}
void voice_ui_render(const voice_navigation_t *n,int playing,int battery,const char *error) {
    if(!screen)return;
    badge_header_battery(battery);
    if(seen_view==(int)n->view&&seen_pack==n->pack&&seen_clip==n->clip&&seen_playing==playing&&seen_volume==n->volume&&seen_battery==battery&&seen_error==error&&seen_theme==badge_theme_revision())return;
    const uint32_t *c=badge_theme_colors();bool changed=seen_view!=(int)n->view||seen_pack!=n->pack||seen_clip!=n->clip;
    lv_obj_set_style_bg_color(screen,lv_color_hex(c[BADGE_BACKGROUND]),0);lv_obj_set_style_bg_color(footer,lv_color_hex(c[BADGE_PANEL]),0);
    color(title,BADGE_ACCENT);color(subtitle,BADGE_TEXT);color(counter,BADGE_MUTED);color(status,BADGE_ACCENT);color(hint,BADGE_MUTED);color(volume_text,BADGE_TEXT);
    bool volume=n->view==VOICE_VOLUME;
    lv_label_set_text(subtitle,volume?"音量设置":n->view==VOICE_PACKS?"音效钥匙扣":voice_packs[n->pack].name);
    if(changed)lv_label_set_long_mode(subtitle,n->view==VOICE_CLIPS?LV_LABEL_LONG_SCROLL_CIRCULAR:LV_LABEL_LONG_CLIP);
    unsigned selected=n->view==VOICE_PACKS?n->pack:n->clip;
    unsigned total=n->view==VOICE_PACKS?VOICE_PACK_COUNT:voice_packs[n->pack].count;
    lv_label_set_text_fmt(counter,volume?"16K MONO":"%u / %u",selected+1,total);
    unsigned first=voice_navigation_first(selected);
    for(unsigned i=0;i<6;i++) {
        unsigned item=first+i;
        if(volume||item>=total){lv_obj_add_flag(rows[i],LV_OBJ_FLAG_HIDDEN);continue;}
        lv_obj_remove_flag(rows[i],LV_OBJ_FLAG_HIDDEN);bool chosen=item==selected;
        lv_obj_set_style_bg_color(rows[i],lv_color_hex(c[chosen?BADGE_PANEL:BADGE_BACKGROUND]),0);
        lv_obj_set_style_border_width(rows[i],chosen?1:0,0);lv_obj_set_style_border_color(rows[i],lv_color_hex(c[BADGE_ACCENT]),0);
        color(names[i],chosen?BADGE_TEXT:BADGE_MUTED);color(numbers[i],chosen?BADGE_ACCENT:BADGE_MUTED);
        if(changed){lv_label_set_long_mode(names[i],chosen?LV_LABEL_LONG_SCROLL_CIRCULAR:LV_LABEL_LONG_DOT);
            lv_label_set_text(names[i],n->view==VOICE_PACKS?voice_packs[item].name:voice_clips[voice_packs[n->pack].first+item].name);}
        bool active=n->view==VOICE_CLIPS&&playing==(int)(voice_packs[n->pack].first+item);
        if(active)lv_label_set_text(numbers[i],">");else lv_label_set_text_fmt(numbers[i],"%u",item+1);
    }
    if(volume){lv_obj_remove_flag(volume_text,LV_OBJ_FLAG_HIDDEN);lv_obj_remove_flag(volume_bar,LV_OBJ_FLAG_HIDDEN);lv_label_set_text_fmt(volume_text,"%d%%",n->volume);lv_bar_set_value(volume_bar,n->volume,LV_ANIM_OFF);}
    else {lv_obj_add_flag(volume_text,LV_OBJ_FLAG_HIDDEN);lv_obj_add_flag(volume_bar,LV_OBJ_FLAG_HIDDEN);}
    lv_obj_set_style_bg_color(volume_bar,lv_color_hex(c[BADGE_PANEL]),LV_PART_MAIN);lv_obj_set_style_bg_color(volume_bar,lv_color_hex(c[BADGE_ACCENT]),LV_PART_INDICATOR);
    if(error)lv_label_set_text(status,error);
    else if(playing>=0)lv_label_set_text_fmt(status,"播放中  音量 %d%%",n->volume);
    else lv_label_set_text_fmt(status,"%s  音量 %d%%",n->view==VOICE_PACKS?VOICE_CLIP_LABEL:"等待播放",n->volume);
    lv_label_set_text(hint,volume?BADGE_HINT_VOLUME:n->view==VOICE_PACKS?BADGE_HINT_VOICE_PACKS:BADGE_HINT_VOICE_CLIPS);
    seen_view=n->view;seen_pack=n->pack;seen_clip=n->clip;seen_playing=playing;seen_volume=n->volume;seen_battery=battery;seen_error=error;seen_theme=badge_theme_revision();
}
void voice_ui_destroy(void) {if(screen){lv_obj_delete(screen);screen=NULL;}}
