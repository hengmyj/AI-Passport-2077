// No LVGL task may see the display until the rounding callback is registered.
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include "../components/bsp/src/bsp_display_lvgl.c"

static lv_display_t display;
static lv_timer_t timer;
static bool timer_fail,panel_sleep_fail,port_resume_fail,task_suspended;
static unsigned brightness=63;
static int task_token;
TaskHandle_t xTaskGetCurrentTaskHandle(void){return &task_token;}
void vTaskSuspend(TaskHandle_t t){assert(t==&task_token);task_suspended=true;}
void vTaskResume(TaskHandle_t t){assert(t==&task_token);task_suspended=false;}
void vTaskDelay(unsigned ms){(void)ms;}
lv_timer_t *lv_timer_create(void (*cb)(lv_timer_t *),unsigned ms,void *u){(void)cb;(void)ms;(void)u;return timer_fail?NULL:&timer;}
void lv_timer_delete(lv_timer_t *t){assert(t==&timer);}
void *lv_display_get_screen_active(lv_display_t *d){return d;}
void lv_obj_invalidate(void *o){(void)o;}
void lv_refr_now(lv_display_t *d){(void)d;}
esp_err_t lvgl_port_stop(void){return ESP_OK;}
esp_err_t lvgl_port_resume(void){return port_resume_fail?ESP_FAIL:ESP_OK;}
uint8_t bsp_display_brightness(void){return brightness;}
void bsp_display_backlight(uint8_t b){brightness=b;}
esp_err_t esp_lcd_panel_disp_sleep(esp_lcd_panel_handle_t h,bool s){(void)h;(void)s;return panel_sleep_fail?ESP_FAIL:ESP_OK;}
esp_err_t esp_lcd_panel_disp_on_off(esp_lcd_panel_handle_t h,bool s){(void)h;(void)s;return ESP_OK;}

static int panel_present = 1, lock_depth, port_live, display_live, callback_live;
static int fail_lock, fail_port, fail_display, fail_event, init_calls, unlocked_flushes;
static int panel_token, io_token;
esp_lcd_panel_handle_t bsp_display_panel(void) { return panel_present ? &panel_token : NULL; }
esp_lcd_panel_io_handle_t bsp_display_io(void) { return &io_token; }
esp_err_t lvgl_port_init(const lvgl_port_cfg_t *cfg) {
    (void)cfg; ++init_calls; assert(!port_live);
    if (fail_port) return ESP_ERR_NO_MEM;
    port_live = 1; return ESP_OK;
}
esp_err_t lvgl_port_deinit(void) {
    // The real API is asynchronous: returning does not release the context.
    assert(false && "Display rollback must not deinit/reinitialize the live port");
    return ESP_OK;
}
bool lvgl_port_lock(uint32_t timeout) {
    (void)timeout; assert(port_live);
    if (fail_lock) return false;
    ++lock_depth; return true;
}
void lvgl_port_unlock(void) {
    assert(lock_depth > 0);
    --lock_depth;
    if (!lock_depth && display_live) {
        assert(callback_live); // Simulate rendering as soon as the lock is free.
        ++unlocked_flushes;
    }
}
lv_display_t *lvgl_port_add_disp(const lvgl_port_display_cfg_t *cfg) {
    assert(lock_depth > 0 && cfg->panel_handle == &panel_token && !display_live);
    assert(lvgl_port_lock(0)); // Real port takes and releases a recursive lock.
    if (!fail_display) display_live = 1;
    lvgl_port_unlock();
    return fail_display ? NULL : &display;
}
esp_err_t lvgl_port_remove_disp(lv_display_t *disp) {
    assert(disp == &display && lock_depth && display_live);
    display_live = callback_live = 0; return ESP_OK;
}
void lv_display_add_event_cb(lv_display_t *disp, void (*cb)(lv_event_t *), int code, void *user) {
    (void)user;
    assert(lock_depth && disp == &display && code == LV_EVENT_FLUSH_START && cb == rounded_flush_event);
    if (!fail_event) callback_live = 1;
}
uint32_t lv_display_get_event_count(lv_display_t *disp) {
    assert(disp == &display && lock_depth);
    return 1 + callback_live; // The display already owns an internal callback.
}
static void expect_failure(void) {
    assert(bsp_lvgl_init() == NULL);
    assert(!s_disp && !lock_depth && !display_live);
    assert(!bsp_lvgl_lock(0));
}
int main(void) {
    panel_present = 0; expect_failure(); panel_present = 1;
    fail_port = 1; expect_failure(); fail_port = 0;
    const int failed_init_calls = init_calls;
    expect_failure(); // A partially initialized port cannot safely be re-created.
    assert(init_calls == failed_init_calls);
    s_port_init_failed = false; // Simulate reboot for remaining scenarios.
    fail_lock = 1; expect_failure(); fail_lock = 0;
    const int retained_init_calls = init_calls;
    fail_display = 1; expect_failure(); fail_display = 0;
    fail_event = 1; expect_failure(); fail_event = 0;
    timer_fail=true;expect_failure();timer_fail=false;
    assert(bsp_lvgl_init() == &display);
    assert(init_calls == retained_init_calls); // Display retries reuse the port.
    assert(callback_live && !lock_depth && unlocked_flushes == 1);
    const int before = init_calls;
    assert(bsp_lvgl_init() == &display && init_calls == before);
    assert(bsp_lvgl_lock(5)); bsp_lvgl_unlock();
    uint16_t pixels[BSP_LCD_W] = {0};
    for (int x = 0; x < BSP_LCD_W; ++x) pixels[x] = 0xffff;
    display.buffer = (lv_draw_buf_t){ .data = (uint8_t *)pixels, .header.stride = sizeof(pixels) };
    lv_area_t area = { .x1 = 0, .y1 = 0, .x2 = BSP_LCD_W - 1, .y2 = 0 };
    lv_event_t ev = { .target = &display, .area = &area };
    rounded_flush_event(&ev);
    assert(pixels[0] == 0 && pixels[BSP_LCD_W - 1] == 0 && pixels[BSP_LCD_W / 2] == 0xffff);
    remember_lvgl_task(&timer);
    assert(bsp_display_suspend()==ESP_OK&&task_suspended&&brightness==0);
    port_resume_fail=true;assert(bsp_display_resume()!=ESP_OK&&task_suspended);
    port_resume_fail=false;assert(bsp_display_resume()==ESP_OK&&!task_suspended&&brightness==63);
    panel_sleep_fail=true;assert(bsp_display_suspend()!=ESP_OK&&bsp_display_is_suspended());
    panel_sleep_fail=false;assert(bsp_display_resume()==ESP_OK&&!task_suspended&&brightness==63);
    puts("BSP LVGL initialization, rollback, suspend/resume retries: PASS");
}
