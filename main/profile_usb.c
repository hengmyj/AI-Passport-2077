#include "profile_usb.h"
#include "badge_power.h"
#include "bsp_display.h"
#include "bsp_pins.h"
#include "profile_protocol.h"
#include "driver/usb_serial_jtag.h"
#include "driver/usb_serial_jtag_vfs.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "cJSON.h"
#include "mbedtls/base64.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

static char line_buffer[4096];
typedef struct { bool started; int64_t deadline; } screen_transfer_t;
static bool screen_write_bytes(const uint8_t *data, size_t size, int64_t deadline) {
    while (size) {
        if (esp_timer_get_time() >= deadline) return false;
        int n = usb_serial_jtag_write_bytes(data, size, pdMS_TO_TICKS(50));
        if (n < 0) return false;
        if (n == 0) { vTaskDelay(1); continue; }
        data += n; size -= (size_t)n;
    }
    return true;
}
static bool screen_write(const uint8_t *data, size_t size, void *context) {
    screen_transfer_t *transfer = context;
    if (!transfer->started) {
        char header[80];
        int len = snprintf(header, sizeof(header), "FAP_SCREENSHOT_V1 %d %d RGB565LE %u\n",
                           BSP_LCD_W, BSP_LCD_H, (unsigned)(BSP_LCD_W * BSP_LCD_H * 2));
        transfer->started = true;
        if (!screen_write_bytes((const uint8_t *)header, (size_t)len, transfer->deadline)) return false;
    }
    return screen_write_bytes(data, size, transfer->deadline);
}
static void capture_screen(void) {
    /* Lock order: LVGL -> stdout. Never wait for LVGL while blocking its logs.
     * USB, not the UI task, owns transport. An unplug bounds the stall to 3 s. */
    if (!bsp_lvgl_lock(1000)) return;
    flockfile(stdout); fflush(stdout);
    screen_transfer_t transfer = {.deadline = esp_timer_get_time() + 3000000};
    esp_err_t result = bsp_display_capture_rgb565(screen_write, &transfer);
    if (result != ESP_OK && !transfer.started) {
        const char error[] = "FAP_SCREENSHOT_ERROR screen_unavailable\n";
        (void)screen_write_bytes((const uint8_t *)error, sizeof(error) - 1, transfer.deadline);
    }
    /* No success marker or log may be appended inside the binary payload. */
    funlockfile(stdout);
    bsp_lvgl_unlock();
}
static void handle(const char *line) {
    if (!strcmp(line, "FAP_SCREENSHOT_V1")) { capture_screen(); return; }
    char *reply=profile_protocol_request(line,true);if(!reply)return;
    flockfile(stdout);fflush(stdout);
    usb_serial_jtag_write_bytes("@BADGE ",7,pdMS_TO_TICKS(1000));
    size_t len=strlen(reply),offset=0;
    while(offset<len){int n=usb_serial_jtag_write_bytes(reply+offset,len-offset,pdMS_TO_TICKS(1000));if(n<=0)break;offset+=n;}
    usb_serial_jtag_write_bytes("\n",1,pdMS_TO_TICKS(1000));funlockfile(stdout);free(reply);
}
static void usb_worker(void *arg) {
    (void)arg;uint8_t data[128];size_t used=0;bool overflow=false;
    for(;;) {
        int n=usb_serial_jtag_read_bytes(data,sizeof(data),pdMS_TO_TICKS(badge_power_screen_off()?1000:50));
        for(int i=0;i<n;i++) {
            if(data[i]=='\n') {
                if(!overflow&&used) {line_buffer[used]=0;handle(line_buffer);}
                used=0;overflow=false;
            } else if(data[i]!='\r') {
                if(used<sizeof(line_buffer)-1)line_buffer[used++]=(char)data[i];else overflow=true;
            }
        }
        profile_protocol_tick();
    }
}
esp_err_t profile_usb_start(void) {
    usb_serial_jtag_driver_config_t cfg={.tx_buffer_size=2048,.rx_buffer_size=4096};
    esp_err_t e=usb_serial_jtag_driver_install(&cfg);if(e!=ESP_OK)return e;
    usb_serial_jtag_vfs_use_driver();
    return xTaskCreate(usb_worker,"profile_usb",6144,NULL,3,NULL)==pdPASS?ESP_OK:ESP_ERR_NO_MEM;
}
