#pragma once
#include <stdint.h>
#include "esp_err.h"
#define ESP_GPIO_WAKEUP_GPIO_LOW 0
esp_err_t esp_deep_sleep_enable_gpio_wakeup(uint64_t,int);
