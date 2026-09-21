#pragma once
#include "esp_err.h"
#include <stdbool.h>
bool bsp_audio_is_sleeping(void);
esp_err_t bsp_audio_sleep(void);
esp_err_t bsp_audio_wake(void);

bool bsp_audio_needs_wake(void);
