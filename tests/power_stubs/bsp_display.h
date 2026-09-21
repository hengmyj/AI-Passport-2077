#pragma once
#include "esp_err.h"
#include <stdint.h>
esp_err_t bsp_display_suspend(void);
esp_err_t bsp_display_resume(void);
uint8_t bsp_display_brightness(void);

bool bsp_display_is_suspended(void);
