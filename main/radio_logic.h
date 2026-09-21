#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
uint16_t radio_parse_frequency(const char *name);
/* Dial position, 0..1000. Unknown frequency is distributed by list index. */
unsigned radio_dial_position(uint16_t frequency, unsigned index, unsigned count);
size_t radio_downmix(int16_t *pcm, size_t bytes, unsigned channels);
bool radio_timer_expired(int64_t deadline, int64_t now);
#ifdef __cplusplus
}
#endif
