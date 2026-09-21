#include "yao_ui.h"
#include "yao_core.h"
#include "badge_header.h"
#include "badge_footer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
LV_FONT_DECLARE(font_xiaozhi_14);
static lv_obj_t *screen,*progress,*calibration,*names[2],*bars[2][6][2],*marks[6],*reader,*text,*hint,*coins[3];
static char cast_time_text[320];
void yao_ui_set_time(int64_t utc,const yao_location_t *location){yao_time_summary(utc,location,cast_time_text,sizeof(cast_time_text));}
static lv_obj_t *label(lv_obj_t *parent,const char *s,int x,int y,int w,unsigned color){
    lv_obj_t *o=lv_label_create(parent);lv_obj_set_pos(o,x,y);lv_obj_set_width(o,w);
    lv_obj_set_style_text_font(o,&font_xiaozhi_14,0);lv_obj_set_style_text_color(o,lv_color_hex(badge_theme_colors()[color]),0);
    lv_label_set_long_mode(o,LV_LABEL_LONG_WRAP);lv_label_set_text(o,s);return o;
}
static void visible(lv_obj_t *o,bool show){if(show)lv_obj_remove_flag(o,LV_OBJ_FLAG_HIDDEN);else lv_obj_add_flag(o,LV_OBJ_FLAG_HIDDEN);}
void yao_ui_create(void){
    yao_ui_set_time(0,NULL);
    const uint32_t *c=badge_theme_colors();screen=lv_obj_create(NULL);lv_obj_remove_style_all(screen);lv_obj_set_size(screen,240,320);
    lv_obj_set_style_bg_opa(screen,LV_OPA_COVER,0);lv_obj_set_style_bg_color(screen,lv_color_hex(c[BADGE_BACKGROUND]),0);
    badge_header_attach(screen);label(screen,"赛博摇卦",12,36,118,BADGE_TEXT);
    progress=label(screen,"",138,37,90,BADGE_ACCENT);lv_obj_set_style_text_align(progress,LV_TEXT_ALIGN_RIGHT,0);
    calibration=label(screen,"",12,53,216,BADGE_MUTED);lv_obj_set_height(calibration,18);lv_label_set_long_mode(calibration,LV_LABEL_LONG_DOT);
    for(unsigned i=0;i<3;i++){
        coins[i]=label(screen,"",57+i*44,76,36,BADGE_ACCENT);
        lv_obj_set_height(coins[i],32);lv_obj_set_style_text_align(coins[i],LV_TEXT_ALIGN_CENTER,0);
        lv_obj_set_style_pad_top(coins[i],5,0);lv_obj_set_style_border_width(coins[i],1,0);
        lv_obj_set_style_border_color(coins[i],lv_color_hex(c[BADGE_ACCENT]),0);lv_obj_set_style_radius(coins[i],16,0);
    }
    for(unsigned h=0;h<2;h++){
        names[h]=label(screen,"",14+h*118,76,108,BADGE_ACCENT);lv_obj_set_height(names[h],36);
        for(unsigned i=0;i<6;i++)for(unsigned half=0;half<2;half++){
            lv_obj_t *o=lv_obj_create(screen);lv_obj_remove_style_all(o);lv_obj_set_pos(o,28+h*120+half*39,193-i*15);lv_obj_set_size(o,33,6);
            lv_obj_set_style_bg_opa(o,LV_OPA_COVER,0);lv_obj_set_style_bg_color(o,lv_color_hex(c[BADGE_TEXT]),0);bars[h][i][half]=o;
        }
    }
    for(unsigned i=0;i<6;i++){marks[i]=label(screen,"",106,186-i*15,22,BADGE_ACCENT);lv_obj_set_style_text_font(marks[i],&font_badge_10,0);}
    reader=lv_obj_create(screen);lv_obj_remove_style_all(reader);lv_obj_set_pos(reader,12,214);lv_obj_set_size(reader,216,79);
    lv_obj_set_scrollbar_mode(reader,LV_SCROLLBAR_MODE_OFF);lv_obj_set_scroll_dir(reader,LV_DIR_VER);
    text=label(reader,"",0,0,214,BADGE_TEXT);lv_obj_set_style_text_line_space(text,4,0);
    hint=badge_footer_create(screen,"");lv_screen_load(screen);
}
void yao_ui_render(const uint8_t lines[6],unsigned count,unsigned coin_bits,uint32_t id,const char *notice){
    if(!screen)return;
    bool complete=count==6;yao_result_t r={0};if(complete&&!yao_calculate(lines,&r))return;
    const char *location_status=yao_location_status();yao_location_t location=yao_location_get();
    if(!strcmp(location_status,"ready"))lv_label_set_text_fmt(calibration,"太阳时 已校准 / %s",location.region);
    else if(!strcmp(location_status,"fetching"))lv_label_set_text(calibration,"太阳时 定位中");
    else if(!strcmp(location_status,"retrying"))lv_label_set_text(calibration,"太阳时 失败 / 15秒后重试");
    else if(!strcmp(location_status,"pending"))lv_label_set_text(calibration,yao_time_valid(yao_time_now())?"太阳时 准备定位":"太阳时 等待校时");
    else lv_label_set_text(calibration,"太阳时 等待 Wi-Fi");
    lv_label_set_text_fmt(progress,complete?"#%lu":"%lu / 6",(unsigned long)(complete?id:count));
    for(unsigned i=0;i<3;i++){visible(coins[i],!complete);lv_label_set_text(coins[i],count?((coin_bits&(1u<<i))?"背":"字"):"钱");}
    for(unsigned h=0;h<2;h++){
        visible(names[h],complete);
        if(complete){char name[32];yao_full_name(h?r.changed:r.original,name,sizeof(name));lv_label_set_text_fmt(names[h],"%s\n%s",h?"之卦":"本卦",name);}
        for(unsigned i=0;i<6;i++){
            bool drawn=i<count&&(complete||!h),yang=(lines[i]&1)!=0;
            if(h&&(lines[i]==6||lines[i]==9))yang=!yang;
            for(unsigned half=0;half<2;half++){
                lv_obj_t *o=bars[h][i][half];visible(o,drawn&&(!yang||!half));
                lv_obj_set_x(o,(complete?28:83)+h*120+half*39);lv_obj_set_width(o,yang?72:33);
                bool moving=lines[i]==6||lines[i]==9;
                lv_obj_set_style_bg_color(o,lv_color_hex(badge_theme_colors()[moving?BADGE_ACCENT:BADGE_TEXT]),0);
                lv_obj_set_style_bg_opa(o,moving?LV_OPA_COVER:LV_OPA_70,0);
            }
            if(!h){visible(marks[i],complete&&(r.moving_mask&(1u<<i)));lv_label_set_text(marks[i],"*");}
        }
    }
    if(notice&&*notice)lv_label_set_text(text,notice);
    else if(!complete)lv_label_set_text_fmt(text,"%s\n%s",count?"按 OK 投掷下一爻":"心中想好所问，按 OK 起卦",count?"自下而上，六次成卦":"字面计二，背面计三");
    else {
        size_t prefix=strlen(cast_time_text),size=prefix+yao_format_full_reading(&r,NULL,0)+1;
        char *body=malloc(size);
        if(body){memcpy(body,cast_time_text,prefix);yao_format_full_reading(&r,body+prefix,size-prefix);lv_label_set_text(text,body);free(body);}
        else lv_label_set_text(text,"内存不足，请返回后重试");
    }
    lv_obj_scroll_to_y(reader,0,LV_ANIM_OFF);
    lv_label_set_text(hint,complete?BADGE_HINT_YAO_RESULT:BADGE_HINT_YAO_CAST);
}
void yao_ui_scroll(int direction){
    if(!reader)return;
    lv_obj_update_layout(reader);
    int32_t max=lv_obj_get_height(text)-lv_obj_get_content_height(reader);if(max<0)max=0;
    int32_t next=lv_obj_get_scroll_y(reader)+(direction<0?-48:48);
    if(next<0)next=0;
    if(next>max)next=max;
    lv_obj_scroll_to_y(reader,next,LV_ANIM_OFF);
}
void yao_ui_destroy(void){if(screen){lv_obj_delete(screen);screen=NULL;}}
