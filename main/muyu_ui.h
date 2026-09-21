#pragma once
#include "muyu_logic.h"
/* All functions require the LVGL lock. Stop both workers before destroy. */
void muyu_ui_create(void);
void muyu_ui_destroy(void);
void muyu_ui_refresh(const muyu_state_t *s, int battery, bool audio, bool storage, bool buttons);
void muyu_ui_hit(bool increased);
void muyu_ui_notice(const char *message);
