#pragma once
#include <stdbool.h>
#include "esp_err.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Shell task owns start/stop/settings. Worker owns microphone and power policy.
 * Stop must succeed before starting ANY other audio producer. */
void xz_wake_init(void);
void xz_wake_on_trigger(void (*notify)(void));
/* Navigation context, called before allocation only under memory pressure. */
void xz_wake_on_prepare(void (*prepare)(void));
bool xz_wake_enabled(void);
bool xz_wake_listening(void);
esp_err_t xz_wake_enable(bool enabled);
esp_err_t xz_wake_stop(void);
void xz_wake_tick(bool audio_available);
bool xz_wake_take_trigger(void);
const char *xz_wake_status(void);
const char *xz_wake_phrase(void);
#ifdef __cplusplus
}
#endif
