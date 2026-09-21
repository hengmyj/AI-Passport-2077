#include "muyu_ui.h"
#include "badge_theme.h"
#include "badge_header.h"
#include "badge_footer.h"
#include "bsp_pins.h"
#include "lvgl.h"

LV_FONT_DECLARE(font_muyu_22);
LV_FONT_DECLARE(font_muyu_14);
LV_FONT_DECLARE(font_badge_10);
static lv_obj_t *screen, *count, *session, *status, *notice;
static lv_obj_t *head, *handle, *feedback, *footer;
static lv_obj_t *text_objects[12];
static unsigned text_roles[12],text_count;
static uint32_t theme_seen;
static lv_point_precise_t handle_points[2];
static lv_timer_t *notice_timer;

static lv_obj_t *label(const char *text, int x, int y, const lv_font_t *font, unsigned role)
{
    lv_obj_t *obj = lv_label_create(screen);
    lv_label_set_text(obj, text);
    lv_obj_set_style_text_font(obj, font, 0);
    lv_obj_set_style_text_color(obj, lv_color_hex(badge_theme_colors()[role]), 0);
    lv_obj_set_pos(obj, x, y);
    LV_ASSERT_MSG(text_count<12,"Too many Muyu labels");
    text_objects[text_count]=obj;text_roles[text_count++]=role;
    return obj;
}

/* Vector artwork uses the same five semantic colors as the badge. */
static void stroke(lv_layer_t *layer,int x1,int y1,int x2,int y2,int width,uint32_t color) {
    lv_draw_line_dsc_t d;lv_draw_line_dsc_init(&d);d.p1=(lv_point_precise_t){x1,y1};d.p2=(lv_point_precise_t){x2,y2};d.width=width;d.color=lv_color_hex(color);d.opa=LV_OPA_COVER;lv_draw_line(layer,&d);
}
static void shape(lv_layer_t *layer,int x,int y,int w,int h,int radius,uint32_t fill,uint32_t border,int width) {
    lv_draw_rect_dsc_t d;lv_draw_rect_dsc_init(&d);d.bg_color=lv_color_hex(fill);d.bg_opa=LV_OPA_COVER;d.border_color=lv_color_hex(border);d.border_width=width;d.radius=radius;
    lv_area_t area={x,y,x+w-1,y+h-1};lv_draw_rect(layer,&d,&area);
}
static void artwork(lv_event_t *event) {
    lv_layer_t *l=lv_event_get_layer(event);const uint32_t *c=badge_theme_colors();
    uint32_t edge=((c[BADGE_ACCENT]&0xfefefe)>>1)+((c[BADGE_PANEL]&0xfefefe)>>1);
    stroke(l,14,251,226,251,1,edge);
    stroke(l,14,61,45,61,2,c[BADGE_ACCENT]);
    for(int i=0;i<4;i++){int y=169+i*18;stroke(l,20,y,26,y,1,edge);stroke(l,214,y,220,y,1,edge);}
    stroke(l,43,242,61,242,1,c[BADGE_MUTED]);stroke(l,43,224,43,242,1,c[BADGE_MUTED]);
    stroke(l,194,157,194,172,1,edge);stroke(l,177,157,194,157,1,edge);
    shape(l,53,168,138,76,38,c[BADGE_PANEL],c[BADGE_ACCENT],2);
    /* The angled opening and twin curved lips identify the percussion body. */
    shape(l,67,179,109,52,26,c[BADGE_BACKGROUND],edge,1);
    shape(l,70,179,104,37,18,c[BADGE_PANEL],c[BADGE_PANEL],0);
    stroke(l,77,219,166,193,7,c[BADGE_BACKGROUND]);
    stroke(l,77,223,166,197,2,c[BADGE_ACCENT]);
    shape(l,158,188,15,15,7,c[BADGE_BACKGROUND],c[BADGE_ACCENT],1);
    stroke(l,98,237,143,237,1,edge);
}
static void apply_theme(void) {
    uint32_t revision=badge_theme_revision();if(theme_seen==revision)return;
    const uint32_t *c=badge_theme_colors();
    lv_obj_set_style_bg_color(screen,lv_color_hex(c[BADGE_BACKGROUND]),0);
    lv_obj_set_style_bg_color(footer,lv_color_hex(c[BADGE_PANEL]),0);
    lv_obj_set_style_bg_color(notice,lv_color_hex(c[BADGE_PANEL]),0);
    lv_obj_set_style_line_color(handle,lv_color_hex(c[BADGE_MUTED]),0);
    lv_obj_set_style_bg_color(head,lv_color_hex(c[BADGE_PANEL]),0);
    lv_obj_set_style_border_color(head,lv_color_hex(c[BADGE_ACCENT]),0);
    for(unsigned i=0;i<text_count;i++)lv_obj_set_style_text_color(text_objects[i],lv_color_hex(c[text_roles[i]]),0);
    theme_seen=revision;lv_obj_invalidate(screen);
}

static void striker_y(void *object, int32_t y)
{
    (void)object;
    handle_points[0] = (lv_point_precise_t){164, y};
    handle_points[1] = (lv_point_precise_t){BSP_LCD_W + 10, y - 34};
    lv_line_set_points_mutable(handle, handle_points, 2);
    lv_obj_set_pos(head, 149, y - 15);
}

static void feedback_progress(void *object, int32_t value)
{
    lv_obj_set_y(object, 148 - value * 34 / 255);
    lv_obj_set_style_text_opa(object, (lv_opa_t)(255 - value), 0);
}

void muyu_ui_hit(bool increased)
{
    /* Reuse both objects: rapid input cannot grow the LVGL heap. */
    lv_label_set_text(feedback, increased ? "功德 + 1" : "功德");
    lv_anim_delete(feedback, feedback_progress);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, feedback);
    lv_anim_set_exec_cb(&a, feedback_progress);
    lv_anim_set_values(&a, 0, 255);
    lv_anim_set_duration(&a, 650);
    lv_anim_start(&a);
    lv_anim_delete(head, striker_y);
    lv_anim_init(&a);
    lv_anim_set_var(&a, head);
    lv_anim_set_exec_cb(&a, striker_y);
    lv_anim_set_values(&a, 133, 180);
    lv_anim_set_duration(&a, 55);
    lv_anim_set_playback_duration(&a, 145);
    lv_anim_start(&a);
}

static void hide_notice(lv_timer_t *timer)
{
    lv_obj_add_flag(notice, LV_OBJ_FLAG_HIDDEN);
    lv_timer_pause(timer);
}

void muyu_ui_notice(const char *message)
{
    lv_label_set_text(notice, message);
    lv_obj_remove_flag(notice, LV_OBJ_FLAG_HIDDEN);
    lv_timer_reset(notice_timer);
    lv_timer_resume(notice_timer);
}

void muyu_ui_create(void)
{
    text_count=0;theme_seen=0;
    screen = lv_obj_create(NULL);
    lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_hex(badge_theme_colors()[BADGE_BACKGROUND]), 0);
    lv_obj_set_style_pad_all(screen, 0, 0);
    lv_obj_set_style_border_width(screen, 0, 0);
    lv_obj_add_event_cb(screen,artwork,LV_EVENT_DRAW_MAIN,NULL);
    badge_header_attach(screen);
    label("敲木鱼",14,39,&font_muyu_14,BADGE_TEXT);
    label("功德",14,69,&font_muyu_14,BADGE_MUTED);
    count=label("0",14,88,&font_muyu_22,BADGE_TEXT);
    session=label("",14,119,&font_muyu_14,BADGE_MUTED);
    handle = lv_line_create(screen);
    lv_obj_set_style_line_width(handle, 9, 0);
    lv_obj_set_style_line_rounded(handle, true, 0);

    head = lv_obj_create(screen);
    lv_obj_remove_flag(head, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(head, 30, 30);
    lv_obj_set_style_radius(head, LV_RADIUS_CIRCLE, 0);

    lv_obj_set_style_border_width(head, 2, 0);
    striker_y(head, 133);
    feedback = label("功德 + 1", 20, 148, &font_muyu_22, BADGE_ACCENT);
    lv_obj_set_style_text_opa(feedback, LV_OPA_TRANSP, 0);
    footer = lv_obj_create(screen);
    lv_obj_remove_flag(footer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(footer, 0, 253);
    lv_obj_set_size(footer, BSP_LCD_W, BSP_LCD_H - 253);
    lv_obj_set_style_radius(footer, 0, 0);
    lv_obj_set_style_border_width(footer, 0, 0);
    lv_obj_set_style_bg_opa(footer, LV_OPA_COVER, 0);
    status = label("", 14, 256, &font_muyu_14, BADGE_TEXT);
    lv_obj_t *hint=badge_footer_create(screen,BADGE_HINT_MUYU);
    text_objects[text_count]=hint;text_roles[text_count++]=BADGE_MUTED;
    notice = label("", 18, 232, &font_muyu_14, BADGE_TEXT);
    lv_obj_set_width(notice, BSP_LCD_W - 36);
    lv_obj_set_style_text_align(notice, LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_set_style_bg_opa(notice, LV_OPA_COVER, 0);
    lv_obj_add_flag(notice, LV_OBJ_FLAG_HIDDEN);
    notice_timer = lv_timer_create(hide_notice, 2400, NULL);
    lv_timer_pause(notice_timer);
    apply_theme();
    lv_screen_load(screen);
}

void muyu_ui_destroy(void)
{
    if (notice_timer) { lv_timer_delete(notice_timer); notice_timer = NULL; }
    if (head) lv_anim_delete(head, striker_y);
    if (feedback) lv_anim_delete(feedback, feedback_progress);
    if (screen) lv_obj_delete(screen);
    screen = count = session = status = notice = NULL;
    head = handle = feedback = footer = NULL;
    text_count=0;theme_seen=0;
}

void muyu_ui_refresh(const muyu_state_t *s, int battery, bool audio, bool storage, bool buttons)
{
    apply_theme();
    static const unsigned bpm[] = {30, 60, 80, 120};
    lv_label_set_text_fmt(count, "%lu", (unsigned long)s->total);
    lv_label_set_text_fmt(session, "本次 %lu  %s", (unsigned long)s->session,
        !storage ? "存储异常" : s->dirty ? "待保存" : "已保存");
    badge_header_battery(battery);
    if (!buttons) lv_label_set_text(status, "按键异常 请重启");
    else lv_label_set_text_fmt(status, "%s %u次/分  %s%u", s->automatic ? "自动" : "手动",
        bpm[s->speed], audio ? "音量" : "无声", audio ? muyu_volume(s) : 0);
}
