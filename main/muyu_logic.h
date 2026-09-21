#pragma once
#include <stdbool.h>
#include <stdint.h>
#define MUYU_COUNT_MAX 999999999U
#define MUYU_SAVE_MS 10000U
#define MUYU_SPEED_COUNT 4U
#define MUYU_VOLUME_COUNT 5U
typedef struct {
    uint32_t total, session, next_hit_ms, last_save_ms;
    uint8_t speed, volume;
    bool automatic, dirty;
} muyu_state_t;
void muyu_init(muyu_state_t *s, uint32_t total, uint8_t speed, uint8_t volume, uint32_t now);
bool muyu_strike(muyu_state_t *s);
void muyu_toggle_auto(muyu_state_t *s, uint32_t now);
void muyu_next_speed(muyu_state_t *s, uint32_t now);
void muyu_next_volume(muyu_state_t *s);
bool muyu_tick(muyu_state_t *s, uint32_t now);
bool muyu_save_due(const muyu_state_t *s, uint32_t now);
void muyu_saved(muyu_state_t *s, uint32_t now, bool success);
uint32_t muyu_interval(const muyu_state_t *s);
uint8_t muyu_volume(const muyu_state_t *s);
