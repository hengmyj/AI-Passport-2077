#pragma once
#include "demo.h"
#include "yao_service.h"
void demo_yao_enter(void);
void demo_yao_exit(void);
void demo_yao_key(bsp_btn_t button,bsp_btn_ev_t event);
esp_err_t demo_yao_start(void);
esp_err_t demo_yao_stop(void);
bool demo_yao_take_interpretation(yao_record_t *record);
void demo_yao_notice(const char *notice);
void demo_yao_tick(void);
