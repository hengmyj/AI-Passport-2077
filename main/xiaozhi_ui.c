#include "xiaozhi_ui.h"
#include "xiaozhi_caption.h"
#include "xiaozhi_face_assets.h"
#include "xiaozhi_face_decoder.h"
#include "xiaozhi_face_transition.h"
#include "xiaozhi_style.h"
#include "badge_theme.h"
#include "badge_header.h"
#include "badge_footer.h"
#include "lvgl.h"
#include <stdio.h>
#include <string.h>
#include <stdatomic.h>
LV_FONT_DECLARE(font_muyu_14);
LV_FONT_DECLARE(font_badge_10);
LV_FONT_DECLARE(font_badge_28);
LV_FONT_DECLARE(font_xiaozhi_14);
static lv_obj_t *screen,*title,*state_label,*detail,*caption,*code,*volume,*hint,*viewport,*heard_view,*heard_label;
static lv_timer_t *refresh;
static atomic_flag pending_lock=ATOMIC_FLAG_INIT;
typedef struct {xz_snapshot_t latest;} mailbox_t;
static mailbox_t *pending;
static bool pending_dirty;
/* All sizable presentation storage lives in the LVGL pool, not the Opus heap. */
typedef struct {
    char source[512],shown[512];
    size_t revealed;
    uint32_t spent,wall_at,wall_credit,audio_elapsed,audio_anchor;
    uint32_t volume_until;
    unsigned last_volume;
    uint32_t heard_at;
    unsigned face,eyes,mouth,level,variant,requested_variant;
    unsigned style,thought_variant;
    xz_face_transition_t transition;
    uint32_t level_until_ms;
    xz_face_t emotion;
    xz_state_t state;
    bool volume_seen,transcript,reply,audio_clock;
} presentation_t;
static presentation_t *ui;
static void text(lv_obj_t *o,const char *s){if(strcmp(lv_label_get_text(o),s))lv_label_set_text(o,s);}
static void hidden(lv_obj_t *o,bool hide){if(hide)lv_obj_add_flag(o,LV_OBJ_FLAG_HIDDEN);else lv_obj_remove_flag(o,LV_OBJ_FLAG_HIDDEN);}
static lv_obj_t *label(lv_obj_t *parent,int x,int y,int w,int h,const lv_font_t *font){
    lv_obj_t *o=lv_label_create(parent);lv_obj_set_pos(o,x,y);lv_obj_set_size(o,w,h);
    lv_label_set_long_mode(o,LV_LABEL_LONG_WRAP);lv_obj_set_style_text_font(o,font,0);
    lv_obj_set_style_text_letter_space(o,0,0);lv_obj_set_style_text_line_space(o,0,0);lv_label_set_text(o,"");return o;
}
bool xiaozhi_ui_post(const xz_snapshot_t *s){
    if(!refresh||!pending||atomic_flag_test_and_set_explicit(&pending_lock,memory_order_acquire))return false;
    pending->latest=*s;pending_dirty=true;atomic_flag_clear_explicit(&pending_lock,memory_order_release);return true;
}
static void type_caption(void){
    if(!ui->transcript)return;
    uint32_t elapsed=ui->audio_clock?ui->audio_elapsed+XZ_CAPTION_LEAD_MS:ui->wall_credit+(uint32_t)(lv_tick_get()-ui->wall_at);
    size_t next=xz_caption_step(ui->source,ui->revealed,elapsed,&ui->spent);
    if(next==ui->revealed)return;
    ui->revealed=next;memcpy(ui->shown,ui->source,next);ui->shown[next]=0;text(caption,ui->shown);
    lv_obj_update_layout(viewport);
    int bottom=lv_obj_get_height(caption)-lv_obj_get_height(viewport);
    lv_obj_scroll_to_y(viewport,bottom>0?bottom:0,LV_ANIM_OFF);
}
static void face_refresh(void);
static void refresh_ui(lv_timer_t *timer){
    (void)timer;xz_snapshot_t next;
    if(atomic_flag_test_and_set_explicit(&pending_lock,memory_order_acquire))return;
    bool dirty=pending_dirty;
    if(dirty){
        next=pending->latest;pending_dirty=false;
    }
    atomic_flag_clear_explicit(&pending_lock,memory_order_release);
    if(dirty)xiaozhi_ui_render(&next);
    if(!lv_obj_has_flag(volume,LV_OBJ_FLAG_HIDDEN)&&(int32_t)(lv_tick_get()-ui->volume_until)>=0)hidden(volume,true);
    face_refresh();
    type_caption();
    if(!lv_obj_has_flag(heard_view,LV_OBJ_FLAG_HIDDEN)){
        int bottom=lv_obj_get_height(heard_label)-lv_obj_get_height(heard_view);
        if(bottom>0){
            /* Read from the beginning, two lines at a time; retain the final
             * page before looping. Reply text/audio have independent clocks. */
            unsigned pages=(bottom+35)/36,step=((uint32_t)(lv_tick_get()-ui->heard_at)/3000)%(pages+2);
            int y=step>=pages?bottom:(int)step*36;
            lv_obj_scroll_to_y(heard_view,y,LV_ANIM_OFF);
        }
    }
}
static void face_refresh(void){
    if(!ui)return;
    uint32_t now=lv_tick_get();
    bool was_armed=ui->transition.armed;
    unsigned face=xz_face_transition(&ui->transition,ui->state,ui->emotion,now);
    if(ui->state==XZ_ACTIVATION)return;
    unsigned variant=ui->requested_variant<XZ_FACE_VARIANTS?ui->requested_variant:0;
    if(!was_armed&&ui->transition.armed)ui->thought_variant=variant;
    if(ui->transition.armed)variant=ui->thought_variant;
    unsigned style=xiaozhi_style_get();
    const xz_face_pose_t *pose=style==XZ_STYLE_ROUND?xz_roadking_pose((xz_face_t)face,variant):style==XZ_STYLE_CUTE?xz_face_pose((xz_face_t)face,variant):xz_portrait_pose((xz_face_t)face,variant);
    bool blink=face==XZ_FACE_NEUTRAL&&ui->state!=XZ_LISTENING&&now%4400<120;
    unsigned eyes=style==XZ_STYLE_ROUND?xz_roadking_eyes((xz_face_t)face,variant,blink):blink?XZ_FACE_BLINK:pose->eyes;
    unsigned phase=xz_face_mouth(ui->state,ui->level,ui->level_until_ms,now);
    unsigned mouth=phase?XZ_FACE_COUNT+phase-1:pose->mouth;
    if(face!=ui->face||variant!=ui->variant||style!=ui->style){ui->face=face;ui->variant=variant;ui->style=style;lv_area_t a={38,60,202,204};lv_obj_invalidate_area(screen,&a);}
    if(eyes!=ui->eyes){ui->eyes=eyes;lv_area_t a={60,110,182,151};lv_obj_invalidate_area(screen,&a);}
    if(mouth!=ui->mouth){ui->mouth=mouth;lv_area_t a={90,149,162,191};lv_obj_invalidate_area(screen,&a);}
}
static void face_part(lv_layer_t *layer,const xz_face_asset_t *asset,int x,int y,uint32_t color,lv_opa_t opacity){
    const lv_image_dsc_t *image=&asset->image;x+=asset->x;y+=asset->y;
    lv_area_t a={x,y,x+image->header.w-1,y+image->header.h-1};
    if(layer->_clip_area.x2<a.x1||layer->_clip_area.x1>a.x2||layer->_clip_area.y2<a.y1||layer->_clip_area.y1>a.y2)return;
    lv_draw_image_dsc_t d;lv_draw_image_dsc_init(&d);d.src=image;d.recolor=lv_color_hex(color);d.recolor_opa=LV_OPA_COVER;d.opa=opacity;
    lv_draw_image(layer,&d,&a);
}
static void face_tears(lv_layer_t *layer,const xz_face_asset_t *asset,int left,int right,int y,uint32_t color){
    /* Crying and laughter both show a matched pair, never a unilateral tear. */
    const int positions[]={left,right};
    for(unsigned i=0;i<2;i++)face_part(layer,asset,positions[i],y,color,LV_OPA_COVER);
}
static void artwork(lv_event_t *e){
    if(!ui||ui->state==XZ_ACTIVATION)return;
    lv_layer_t *layer=lv_event_get_layer(e);const uint32_t *c=badge_theme_colors();
    if(ui->style==XZ_STYLE_ROUND){
        /* Preserve reference proportions while deriving every layer from the
         * badge palette. Small features use independent dirty areas. */
        const xz_face_pose_t *p=xz_roadking_pose((xz_face_t)ui->face,ui->variant);
        lv_color_t bgc=lv_color_hex(c[BADGE_BACKGROUND]),textc=lv_color_hex(c[BADGE_TEXT]),accent=lv_color_hex(c[BADGE_ACCENT]);
        lv_color_t skin=lv_color_mix(lv_color_mix(textc,accent,205),bgc,218);
        int light=lv_color_brightness(skin),bg_light=lv_color_brightness(bgc),text_light=lv_color_brightness(textc);
        int bg_delta=light>bg_light?light-bg_light:bg_light-light,text_delta=light>text_light?light-text_light:text_light-light;
        lv_color_t ink=bg_delta>text_delta?bgc:textc;
        uint32_t line_color=lv_color_to_u32(ink);
        face_part(layer,&xz_round_outline,40,62,line_color,LV_OPA_COVER);
        face_part(layer,&xz_round_hair,40,62,lv_color_to_u32(lv_color_mix(ink,skin,224)),LV_OPA_COVER);
        face_part(layer,&xz_round_skin,40,62,lv_color_to_u32(skin),LV_OPA_COVER);
        face_part(layer,&xz_round_nose,40,62,lv_color_to_u32(lv_color_mix(ink,skin,150)),LV_OPA_COVER);
        face_part(layer,&xz_round_shirt,40,62,lv_color_to_u32(lv_color_mix(accent,ink,65)),LV_OPA_COVER);
        face_part(layer,&xz_round_eye_left_frames[ui->eyes],64+p->eye_x,120,line_color,LV_OPA_COVER);
        face_part(layer,&xz_round_eye_right_frames[ui->eyes],64+p->eye_x,120,line_color,LV_OPA_COVER);
        face_part(layer,&xz_round_mouth_frames[ui->mouth],93+p->mouth_x,163,lv_color_to_u32(lv_color_mix(ink,skin,205)),LV_OPA_COVER);
        if(ui->face==XZ_FACE_EMBARRASSED||ui->face==XZ_FACE_LOVING){
            uint32_t blush=lv_color_to_u32(lv_color_mix(accent,ink,170));
            face_part(layer,&xz_face_decor[XZ_DECOR_CHEEK],76,148,blush,LV_OPA_40);
            face_part(layer,&xz_face_decor[XZ_DECOR_CHEEK],153,148,blush,LV_OPA_40);
        }
        if(ui->face==XZ_FACE_CRYING||(ui->face==XZ_FACE_FUNNY&&ui->variant==2)){
            uint32_t tear=lv_color_to_u32(lv_color_mix(accent,ink,170));
            face_tears(layer,&xz_portrait_tear,81+p->eye_x,149+p->eye_x,140,tear);
        }
        if(ui->face==XZ_FACE_LOVING||ui->face==XZ_FACE_KISSY)face_part(layer,&xz_face_decor[XZ_DECOR_HEART],179,83,c[BADGE_ACCENT],LV_OPA_80);
        if(ui->face==XZ_FACE_CONFUSED)face_part(layer,&xz_face_decor[XZ_DECOR_QUESTION],181,149,c[BADGE_ACCENT],LV_OPA_80);
        if(ui->face==XZ_FACE_SLEEPY)face_part(layer,&xz_face_decor[XZ_DECOR_ZZZ],181,83,c[BADGE_ACCENT],LV_OPA_80);
        return;
    }
    bool female=ui->style==XZ_STYLE_FEMALE;
    bool abstract=ui->style==XZ_STYLE_ABSTRACT||female;
    lv_color_t body=lv_color_mix(lv_color_hex(c[BADGE_TEXT]),lv_color_hex(c[BADGE_ACCENT]),female?242:abstract?205:40);
    int brightness=lv_color_brightness(body),bg=lv_color_brightness(lv_color_hex(c[BADGE_BACKGROUND])),fg=lv_color_brightness(lv_color_hex(c[BADGE_TEXT]));
    int bg_diff=brightness>bg?brightness-bg:bg-brightness,fg_diff=brightness>fg?brightness-fg:fg-brightness;
    uint32_t ink=c[bg_diff>fg_diff?BADGE_BACKGROUND:BADGE_TEXT];
    const xz_face_pose_t *pose=abstract?xz_portrait_pose((xz_face_t)ui->face,ui->variant):xz_face_pose((xz_face_t)ui->face,ui->variant);
    face_part(layer,female?&xz_female_body:abstract?&xz_abstract_body:&xz_face_body,48,65,lv_color_to_u32(body),LV_OPA_COVER);
    if(abstract)face_part(layer,female?&xz_female_detail:&xz_abstract_detail,48,65,ink,LV_OPA_COVER);
    if(abstract&&!female)face_part(layer,&xz_bald_gloss,48,65,c[BADGE_TEXT],LV_OPA_COVER);
    if(female)face_part(layer,&xz_female_accent,48,65,c[BADGE_ACCENT],LV_OPA_COVER);
    face_part(layer,abstract?&xz_abstract_eye_frames[ui->eyes]:&xz_face_eye_frames[ui->eyes],80+pose->eye_x+(abstract?4:0),113,ink,LV_OPA_COVER);
    face_part(layer,abstract?&xz_abstract_mouth_frames[ui->mouth]:&xz_face_mouth_frames[ui->mouth],96+pose->mouth_x+(abstract?4:0),152,ink,LV_OPA_COVER);
    unsigned face=ui->face;
    unsigned variant=ui->variant;int accent_x=variant==1?44:176,accent_y=variant==2?79:90;
    lv_opa_t blush=abstract?LV_OPA_20:(face==XZ_FACE_EMBARRASSED||face==XZ_FACE_LOVING||face==XZ_FACE_KISSY)?LV_OPA_40:LV_OPA_20;
    if(!abstract||face==XZ_FACE_EMBARRASSED||face==XZ_FACE_LOVING){
        face_part(layer,&xz_face_decor[XZ_DECOR_CHEEK],abstract?88:64,149,ink,blush);
        face_part(layer,&xz_face_decor[XZ_DECOR_CHEEK],abstract?153:162,149,ink,blush);
    }
    if(face==XZ_FACE_CRYING||(face==XZ_FACE_FUNNY&&(!abstract||variant==2))){
        uint32_t tear=abstract?lv_color_to_u32(lv_color_mix(lv_color_hex(c[BADGE_ACCENT]),lv_color_hex(ink),128)):c[BADGE_TEXT];
        face_tears(layer,abstract?&xz_portrait_tear:&xz_face_decor[XZ_DECOR_TEAR],
                   (abstract?101:90)+pose->eye_x,(abstract?137:138)+pose->eye_x,138,tear);
    }
    if(face==XZ_FACE_LOVING||face==XZ_FACE_KISSY||(face==XZ_FACE_WINKING&&variant==2)){
        face_part(layer,&xz_face_decor[XZ_DECOR_HEART],accent_x,accent_y,c[BADGE_ACCENT],LV_OPA_COVER);
        if(variant==2)face_part(layer,&xz_face_decor[XZ_DECOR_HEART],44,90,c[BADGE_ACCENT],LV_OPA_60);
    }
    if((face==XZ_FACE_EMBARRASSED&&!abstract)||face==XZ_FACE_SHOCKED)
        face_tears(layer,&xz_face_decor[XZ_DECOR_SWEAT],abstract?60:56,abstract?176:172,
                   variant==2?111:116,c[BADGE_TEXT]);
    if(face==XZ_FACE_CONFUSED)face_part(layer,&xz_face_decor[XZ_DECOR_QUESTION],accent_x,accent_y,c[BADGE_ACCENT],LV_OPA_COVER);
    if(face==XZ_FACE_SLEEPY)face_part(layer,&xz_face_decor[XZ_DECOR_ZZZ],accent_x,accent_y,c[BADGE_ACCENT],LV_OPA_COVER);
    if(face==XZ_FACE_COOL||face==XZ_FACE_CONFIDENT){
        face_part(layer,&xz_face_decor[XZ_DECOR_STAR],accent_x,accent_y,c[BADGE_ACCENT],LV_OPA_COVER);
        if(variant==2)face_part(layer,&xz_face_decor[XZ_DECOR_STAR],44,92,c[BADGE_ACCENT],LV_OPA_60);
    }
    if(face==XZ_FACE_THINKING)face_part(layer,&xz_face_decor[abstract?XZ_DECOR_QUESTION:XZ_DECOR_HAND],abstract?174:variant==1?88:130,abstract?157:163,abstract?c[BADGE_ACCENT]:ink,LV_OPA_70);
}
void xiaozhi_ui_create(void){
    face_decoder_start();
    screen=lv_obj_create(NULL);lv_obj_remove_style_all(screen);lv_obj_set_size(screen,240,320);
    lv_obj_remove_flag(screen,LV_OBJ_FLAG_SCROLLABLE);lv_obj_set_style_bg_opa(screen,LV_OPA_COVER,0);
    badge_header_attach(screen);lv_obj_add_event_cb(screen,artwork,LV_EVENT_DRAW_MAIN,NULL);
    title=label(screen,16,39,120,14,&font_badge_10);text(title,"XIAOZHI");
    volume=label(screen,171,39,53,14,&font_badge_10);lv_obj_set_style_text_align(volume,LV_TEXT_ALIGN_RIGHT,0);
    state_label=label(screen,16,217,208,18,&font_muyu_14);lv_obj_set_style_text_align(state_label,LV_TEXT_ALIGN_CENTER,0);
    viewport=lv_obj_create(screen);lv_obj_remove_style_all(viewport);lv_obj_set_pos(viewport,22,248);lv_obj_set_size(viewport,196,36);
    lv_obj_remove_flag(viewport,LV_OBJ_FLAG_SCROLLABLE);lv_obj_set_scrollbar_mode(viewport,LV_SCROLLBAR_MODE_OFF);
    caption=label(viewport,0,0,196,LV_SIZE_CONTENT,&font_xiaozhi_14);lv_obj_set_style_text_align(caption,LV_TEXT_ALIGN_LEFT,0);
    heard_view=lv_obj_create(screen);lv_obj_remove_style_all(heard_view);lv_obj_set_pos(heard_view,22,224);lv_obj_set_size(heard_view,196,36);
    lv_obj_remove_flag(heard_view,LV_OBJ_FLAG_SCROLLABLE);lv_obj_set_scrollbar_mode(heard_view,LV_SCROLLBAR_MODE_OFF);
    heard_label=label(heard_view,0,0,196,LV_SIZE_CONTENT,&font_xiaozhi_14);
    detail=label(screen,22,248,196,36,&font_xiaozhi_14);lv_obj_set_style_text_align(detail,LV_TEXT_ALIGN_CENTER,0);
    code=label(screen,16,128,208,36,&font_badge_28);lv_obj_set_style_text_align(code,LV_TEXT_ALIGN_CENTER,0);
    hint=badge_footer_create(screen,"");
    ui=lv_malloc(sizeof(*ui));if(ui){memset(ui,0,sizeof(*ui));ui->style=xiaozhi_style_get();}
    pending=lv_malloc(sizeof(*pending));if(pending)memset(pending,0,sizeof(*pending));pending_dirty=false;
    hidden(viewport,true);hidden(heard_view,true);hidden(code,true);hidden(volume,true);
    if(ui){refresh=lv_timer_create(refresh_ui,100,NULL);}
    lv_screen_load(screen);
}
void xiaozhi_ui_render(const xz_snapshot_t *s){
    if(!screen||!ui){return;}const uint32_t *c=badge_theme_colors();
    bool recolor=!lv_color_eq(lv_obj_get_style_bg_color(screen,0),lv_color_hex(c[BADGE_BACKGROUND]));
    if(recolor)lv_obj_set_style_bg_color(screen,lv_color_hex(c[BADGE_BACKGROUND]),0);
    lv_obj_t *objects[]={title,state_label,detail,caption,code,volume,hint};
    unsigned colors[]={BADGE_MUTED,BADGE_ACCENT,BADGE_TEXT,BADGE_TEXT,BADGE_ACCENT,BADGE_MUTED,BADGE_MUTED};
    for(unsigned i=0;i<7;i++)if(!lv_color_eq(lv_obj_get_style_text_color(objects[i],0),lv_color_hex(c[colors[i]]))){lv_obj_set_style_text_color(objects[i],lv_color_hex(c[colors[i]]),0);recolor=true;}
    if(recolor)lv_obj_invalidate(screen);
    if(!lv_color_eq(lv_obj_get_style_text_color(heard_label,0),lv_color_hex(c[BADGE_MUTED])))
        lv_obj_set_style_text_color(heard_label,lv_color_hex(c[BADGE_MUTED]),0);
    xz_state_t previous=ui->state;ui->state=s->state;
    if(refresh&&previous!=s->state)lv_timer_set_period(refresh,(s->state==XZ_LISTENING||s->state==XZ_SPEAKING||s->state==XZ_THINKING)?50:100);
    const char *names[]={"随时开口","正在连接","等待绑定","对话已暂停","正在听你说","让我想一想","小智在回应","连接提示"};
    text(state_label,names[s->state]);text(detail,s->detail);text(code,s->code);
    const char *reply_text=xz_caption_is_tool_progress(s->text)?"":s->text;
    /* Keep the last recognized input until the next recognition, including
     * automatic re-listening. Never replace it with the response. */
    if(s->heard[0]&&strcmp(lv_label_get_text(heard_label),s->heard)){
        text(heard_label,s->heard);ui->heard_at=lv_tick_get();
        lv_obj_update_layout(heard_view);lv_obj_scroll_to_y(heard_view,0,LV_ANIM_OFF);
    }
    bool conversation=s->state==XZ_THINKING||s->state==XZ_SPEAKING||s->state==XZ_LISTENING||s->state==XZ_READY;
    bool have_heard=conversation&&lv_label_get_text(heard_label)[0];
    hidden(heard_view,!have_heard);
    lv_obj_set_y(viewport,have_heard?262:248);lv_obj_set_height(viewport,36);
    bool transcript=conversation&&reply_text[0];
    hidden(viewport,!transcript);hidden(detail,transcript||have_heard);hidden(code,s->state!=XZ_ACTIVATION);
    lv_obj_set_y(state_label,s->state==XZ_ACTIVATION?72:have_heard?204:217);
    lv_obj_set_y(detail,s->state==XZ_ACTIVATION?190:s->state==XZ_ERROR?239:248);
    lv_obj_set_height(detail,s->state==XZ_ACTIVATION?72:s->state==XZ_ERROR?54:36);
    if(transcript){
        bool reply=true;
        const char *source=reply_text;
        bool audio=reply&&s->state==XZ_SPEAKING;
        if(!ui->transcript||reply!=ui->reply||strcmp(ui->source,source)||(audio&&s->caption_start_ms!=ui->audio_anchor)){
            snprintf(ui->source,sizeof(ui->source),"%s",source);ui->shown[0]=0;ui->revealed=0;ui->spent=0;
            ui->wall_at=lv_tick_get();ui->wall_credit=0;ui->audio_clock=audio;ui->audio_anchor=s->caption_start_ms;
            ui->revealed=xz_caption_step(ui->source,0,XZ_CAPTION_LEAD_MS,&ui->spent);
            memcpy(ui->shown,ui->source,ui->revealed);ui->shown[ui->revealed]=0;
            text(caption,ui->shown);lv_obj_update_layout(viewport);
            int bottom=lv_obj_get_height(caption)-lv_obj_get_height(viewport);lv_obj_scroll_to_y(viewport,bottom>0?bottom:0,LV_ANIM_OFF);
        }else if(ui->audio_clock&&!audio){
            ui->audio_clock=false;ui->wall_at=lv_tick_get();ui->wall_credit=ui->spent;
        }
        ui->reply=reply;ui->audio_elapsed=s->played_ms>=ui->audio_anchor?s->played_ms-ui->audio_anchor:0;
    }
    ui->transcript=transcript;
    char v[16];snprintf(v,sizeof(v),"VOL %u",s->volume);text(volume,v);
    if(ui->volume_seen&&ui->last_volume!=s->volume){ui->volume_until=lv_tick_get()+3000;hidden(volume,false);}
    ui->last_volume=s->volume;ui->volume_seen=true;
    text(hint,s->state==XZ_LISTENING?BADGE_HINT_XZ_PAUSE:s->state==XZ_ACTIVATION||s->state==XZ_ERROR?BADGE_HINT_XZ_RETRY:s->state==XZ_CONNECTING?BADGE_HINT_XZ_WAIT:s->state==XZ_SPEAKING||s->state==XZ_THINKING?BADGE_HINT_XZ_INTERRUPT:BADGE_HINT_XZ_START);
    ui->emotion=s->emotion;ui->requested_variant=s->face_variant;ui->level=s->level;ui->level_until_ms=s->level_until_ms;
    face_refresh();
    if((previous==XZ_ACTIVATION)!=(s->state==XZ_ACTIVATION)){lv_area_t a={26,56,214,209};lv_obj_invalidate_area(screen,&a);}
}
void xiaozhi_ui_destroy(void){
    if(refresh){lv_timer_delete(refresh);refresh=NULL;}pending_dirty=false;lv_free(pending);pending=NULL;
    lv_free(ui);ui=NULL;viewport=NULL;if(screen){lv_obj_delete(screen);screen=NULL;}
    face_decoder_stop();heard_view=heard_label=NULL;
}
