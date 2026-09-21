#pragma once
#include "esp_err.h"
#include <stdbool.h>
esp_err_t profile_protocol_init(void);
/* Caller owns returned JSON and must free it. All storage mutations serialized. */
char *profile_protocol_request(const char *request,bool usb);
void profile_protocol_tick(void);

/* UI callers hold LVGL first, then this lock; protocol never takes LVGL. */
bool profile_protocol_lock(void);
void profile_protocol_unlock(void);
esp_err_t profile_protocol_select(unsigned id);
esp_err_t profile_protocol_edit(unsigned id,unsigned revision,unsigned field,const char *text);
