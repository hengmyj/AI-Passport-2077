#pragma once
#include <stdint.h>
/* Shared by badge shell and built-in games. Hold the LVGL lock for all access. */
enum { BADGE_BACKGROUND, BADGE_PANEL, BADGE_ACCENT, BADGE_TEXT, BADGE_MUTED, BADGE_COLOR_COUNT };
void badge_theme_set(const uint32_t colors[BADGE_COLOR_COUNT]);
const uint32_t *badge_theme_colors(void);
uint32_t badge_theme_revision(void);
