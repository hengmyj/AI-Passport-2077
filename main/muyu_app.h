#pragma once
#include "bsp_button.h"
void demo_muyu_enter(void);
void demo_muyu_exit(void);
void demo_muyu_key(bsp_btn_t button, bsp_btn_ev_t event);
esp_err_t demo_muyu_start(void);
esp_err_t demo_muyu_stop(void);
