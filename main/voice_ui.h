#pragma once
#include "voice_navigation.h"
void voice_ui_create(void);
void voice_ui_destroy(void);
/* All UI calls require the LVGL lock. -1 playing means idle, error is visible. */
void voice_ui_render(const voice_navigation_t *navigation,int playing,int battery,const char *error);
