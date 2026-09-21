// components/bsp/src/bsp_display_lvgl.c
// LVGL 接入单独成文件:不用 LVGL 的开发者删掉本文件 + idf_component.yml 里的两条依赖即可。
#include "bsp_display.h"
#include "bsp_display_rounding.h"
#include "bsp_capture_stream.h"
#include "bsp_pins.h"
#include "esp_lvgl_port.h"
#include "esp_log.h"
#include "esp_lcd_panel_ops.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "bsp_lvgl";

static lv_display_t *s_disp;
static bool s_port_initialized, s_port_init_failed;
static TaskHandle_t s_lvgl_task;
static bool s_suspended;
static bsp_capture_stream_t *s_capture; /* Only accessed under the LVGL lock. */
static uint8_t s_saved_brightness=80;
static void remember_lvgl_task(lv_timer_t *timer) {
    s_lvgl_task=xTaskGetCurrentTaskHandle();lv_timer_delete(timer);
}

static void rounded_flush_event(lv_event_t *event)
{
    lv_display_t *disp = lv_event_get_target(event);
    const lv_area_t *area = lv_event_get_param(event);
    lv_draw_buf_t *draw_buf = lv_display_get_buf_active(disp);
    if (!area || !draw_buf || !draw_buf->data ||
        lv_display_get_color_format(disp) != LV_COLOR_FORMAT_RGB565) {
        return;
    }

    const int32_t width = lv_area_get_width(area);
    if (draw_buf->header.stride < (uint32_t)width * sizeof(uint16_t)) return;

    for (int32_t y = area->y1; y <= area->y2; ++y) {
        if (y >= BSP_LVGL_SCREEN_RADIUS &&
            y < BSP_LCD_H - BSP_LVGL_SCREEN_RADIUS) {
            continue;
        }

        uint16_t *row = (uint16_t *)(draw_buf->data +
                                     (y - area->y1) * draw_buf->header.stride);
        for (int32_t x = area->x1; x <= area->x2; ++x) {
            if (bsp_display_pixel_outside_rounded_rect(
                    x, y, BSP_LCD_W, BSP_LCD_H, BSP_LVGL_SCREEN_RADIUS)) {
                // The port swaps RGB565 bytes after this event; black is 0 in
                // either byte order, so masking here is safe.
                row[x - area->x1] = 0;
            }
        }
    }
    if (s_capture) {
        bsp_capture_stream_strip(s_capture, area->x1, area->y1, area->x2, area->y2,
                                 draw_buf->data, draw_buf->header.stride);
    }
}

esp_err_t bsp_display_capture_rgb565(bsp_display_capture_write_fn write, void *context) {
    if (!s_disp || s_suspended || s_capture || !write ||
        lv_display_get_color_format(s_disp) != LV_COLOR_FORMAT_RGB565) {
        return ESP_ERR_INVALID_STATE;
    }
    bsp_capture_stream_t capture = {
        .width = BSP_LCD_W, .height = BSP_LCD_H, .write = write, .context = context
    };
    /* Invalidate the full screen so the existing partial renderer emits every
     * row once. These exact masked pixels also reach the panel after byte swap.
     * Holding the port lock freezes page mutations for the entire frame. */
    lv_obj_invalidate(lv_display_get_screen_active(s_disp));
    s_capture = &capture;
    lv_refr_now(s_disp);
    s_capture = NULL;
    return bsp_capture_stream_complete(&capture) ? ESP_OK : ESP_FAIL;
}

lv_display_t *bsp_lvgl_init(void) {
    if (s_disp) return s_disp;
    if (!bsp_display_panel()) {
        ESP_LOGE(TAG, "请先成功调用 bsp_display_init()");
        return NULL;
    }

    if (!s_port_initialized) {
        // Port 2.9.0 has no public completion handshake for asynchronous deinit.
        // Never overwrite a possibly live context after a partial port failure.
        if (s_port_init_failed) {
            ESP_LOGE(TAG, "LVGL port 初始化未完成，需重启后重试");
            return NULL;
        }
        const lvgl_port_cfg_t pc = ESP_LVGL_PORT_INIT_CONFIG();
        if (lvgl_port_init(&pc) != ESP_OK) {
            s_port_init_failed = true;
            ESP_LOGE(TAG, "lvgl_port_init 失败，需重启后重试");
            return NULL;
        }
        s_port_initialized = true;
    }

    const lvgl_port_display_cfg_t dc = {
        .panel_handle = bsp_display_panel(),
        .io_handle    = bsp_display_io(),
        // ⚠ C3 无 PSRAM,DMA 只能用内部 RAM(总共约 150KB)。
        // 20 行单缓冲 ≈ 9.6KB;若改成 40 行双缓冲(≈37.5KB)会把 I2S 等外设的
        // DMA 描述符挤到 NO_MEM。刷新略慢但稳。
        // Ten rows leave 4,800 more contiguous bytes for TLS + Opus on C3.
        .buffer_size   = (uint32_t)BSP_LCD_W * 10,
        .double_buffer = false,
        .hres = BSP_LCD_W, .vres = BSP_LCD_H,
        // 旋转/镜像必须在这里配:esp_lvgl_port 注册显示时会重新下发 MADCTL,
        // 覆盖 bsp_display.c 里 esp_lcd_panel_mirror() 的设置。
        .rotation = { .swap_xy = false, .mirror_x = false, .mirror_y = false },
        // swap_bytes:LVGL 输出小端 RGB565,ST7789 走 SPI 要大端 → 需交换高低字节。
        .flags = { .buff_dma = true, .swap_bytes = true },
    };
    // The port mutex is recursive. Keep registration and the mask callback in
    // one critical section, before the new display can produce its first flush.
    if (!lvgl_port_lock(0)) {
        ESP_LOGE(TAG, "LVGL 初始化加锁失败");
        return NULL;
    }
    lv_display_t *disp = lvgl_port_add_disp(&dc);
    bool mask_registered = false;
    if (disp) {
        // LVGL 9.5 returns void here; check the list while still holding the lock.
        const uint32_t count = lv_display_get_event_count(disp);
        lv_display_add_event_cb(disp, rounded_flush_event, LV_EVENT_FLUSH_START, NULL);
        mask_registered = lv_display_get_event_count(disp) == count + 1;
    }
    if (!mask_registered) {
        ESP_LOGE(TAG, "LVGL display 或圆角回调注册失败");
        if (disp) lvgl_port_remove_disp(disp);
        lvgl_port_unlock();
        // Retain the initialized port for retry. Deinit is asynchronous and can
        // race the next init (or even run before the task sets running=true).
        return NULL;
    }

    // Mask the final RGB565 flush instead of using root-screen clip_corner.
    // Full-screen rounded clipping creates an ARGB layer that does not fit the
    // 24 KB LVGL pool reliably on this no-PSRAM target.
    lv_timer_t *task_timer = lv_timer_create(remember_lvgl_task, 1, NULL);
    if (!task_timer) {
        lvgl_port_remove_disp(disp);
        lvgl_port_unlock();
        return NULL;
    }
    s_disp = disp;
    lvgl_port_unlock();

    ESP_LOGI(TAG, "LVGL 就绪，全局圆角=%d，外部填充=黑色", BSP_LVGL_SCREEN_RADIUS);
    return s_disp;
}

bool bsp_lvgl_lock(int timeout_ms) {
    if (!s_disp) return false;
    return lvgl_port_lock(timeout_ms);
}
void bsp_lvgl_unlock(void) {
    if (s_disp) lvgl_port_unlock();
}

esp_err_t bsp_display_suspend(void) {
    if(s_suspended)return ESP_OK;
    if(!s_disp||!s_lvgl_task)return ESP_ERR_INVALID_STATE;
    if(!bsp_lvgl_lock(1000))return ESP_ERR_TIMEOUT;
    s_saved_brightness=bsp_display_brightness();
    if(!s_saved_brightness)s_saved_brightness=80;
    esp_err_t e=lvgl_port_stop();
    if(e!=ESP_OK){lvgl_port_resume();bsp_lvgl_unlock();return e;}
    /* In this LVGL version the disabled handler returns 1 ms. Stop alone
     * would increase wakeups. With its port lock held, the LVGL task cannot
     * own any UI/flush work, so it is safe to suspend it as well. */
    vTaskSuspend(s_lvgl_task);s_suspended=true;
    bsp_display_backlight(0);
    e=esp_lcd_panel_disp_on_off(bsp_display_panel(),false);
    if(e==ESP_OK)e=esp_lcd_panel_disp_sleep(bsp_display_panel(),true);
    bsp_lvgl_unlock();
    if(e!=ESP_OK)(void)bsp_display_resume();
    return e;
}
esp_err_t bsp_display_resume(void) {
    if(!s_suspended)return ESP_OK;
    if(!bsp_lvgl_lock(1000))return ESP_ERR_TIMEOUT;
    esp_err_t e=esp_lcd_panel_disp_sleep(bsp_display_panel(),false);
    if(e==ESP_OK){vTaskDelay(pdMS_TO_TICKS(120));e=esp_lcd_panel_disp_on_off(bsp_display_panel(),true);}
    if(e==ESP_OK)e=lvgl_port_resume();
    if(e==ESP_OK){
        lv_obj_invalidate(lv_display_get_screen_active(s_disp));
        vTaskResume(s_lvgl_task);s_suspended=false;
        bsp_display_backlight(s_saved_brightness?s_saved_brightness:80);
    }
    bsp_lvgl_unlock();return e;
}

bool bsp_display_is_suspended(void) { return s_suspended; }
