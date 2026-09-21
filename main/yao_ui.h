#pragma once
#include <stdint.h>
#include "yao_time.h"
/* All UI calls require the LVGL lock. */
void yao_ui_create(void);
void yao_ui_set_time(int64_t utc,const yao_location_t *location);
void yao_ui_destroy(void);
void yao_ui_render(const uint8_t lines[6],unsigned count,unsigned coin_bits,uint32_t id,const char *notice);
void yao_ui_scroll(int direction);
