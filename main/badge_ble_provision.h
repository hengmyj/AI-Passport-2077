#pragma once

#include "esp_err.h"
#include <stdbool.h>

typedef struct {
    bool active;
    bool connected;
    char name[32];
    char status[96];
    int last_error;
} badge_ble_provision_status_t;

esp_err_t badge_ble_provision_start(void);
esp_err_t badge_ble_provision_stop(void);
void badge_ble_provision_status(badge_ble_provision_status_t *out);

