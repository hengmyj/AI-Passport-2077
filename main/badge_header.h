#pragma once
#include "lvgl.h"
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Shared home/mini-app header. All calls require the LVGL lock. */
void badge_header_attach(lv_obj_t *screen);
void badge_header_battery(int percent);
void badge_header_network(bool connected);
void badge_header_badge(unsigned active,unsigned count);
void badge_header_refresh(void);
#ifdef __cplusplus
}
#endif
