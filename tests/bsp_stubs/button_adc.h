#pragma once
#include "iot_button.h"
#include "esp_adc/adc_oneshot.h"
typedef struct {adc_oneshot_unit_handle_t *adc_handle;int unit_id,adc_channel,button_index,min,max;} button_adc_config_t;
esp_err_t iot_button_new_adc_device(const button_config_t *,const button_adc_config_t *,button_handle_t *);
