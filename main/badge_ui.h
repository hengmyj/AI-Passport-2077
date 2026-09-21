#pragma once
#include "badge_navigation.h"
#include <stdbool.h>
#include <stdint.h>
/* LVGL lock required. This screen is retained only while in the badge shell. */
void badge_ui_create(void);
void badge_ui_destroy(void);
void badge_ui_set_profile(const uint8_t *pixels);
void badge_ui_render(const badge_navigation_t *state, const char *game_name,
                     const char *description, const char *category);
void badge_ui_status(const char *unit, int battery, uint32_t uptime_seconds, bool storage, bool input);

void badge_ui_set_custom(const uint8_t *brand,const uint8_t *logo,const uint8_t *qr,const uint32_t colors[5]);
void badge_ui_network(bool active,bool connected,const char *sta_ssid,const char *ap_ssid,const char *password,const char *ip,const char *message);

void badge_ui_set_badges(const uint8_t *const *cards,unsigned count);
/* Top-layer chooser also works above games. NULL dismisses it. */
void badge_ui_return_menu(const badge_return_menu_t *menu);

void badge_ui_set_portrait(const uint8_t *pixels,const uint8_t *avatar);
void badge_ui_set_badge_formats(const unsigned versions[5]);

void badge_ui_set_tactical(const uint8_t *pixels,const uint8_t *avatar);

void badge_ui_ai_status(const char *text);
#ifdef BADGE_CONTROL_DEVICE_PROBE
unsigned badge_ui_probe_home_count(void);
unsigned badge_ui_probe_home_badge(void);
#endif
