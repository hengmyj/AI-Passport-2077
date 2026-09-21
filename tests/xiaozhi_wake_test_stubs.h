#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"
#define ESP_ERR_TIMEOUT 0x107
#define BADGE_WAKE_PHRASE "test phrase"
#define BADGE_WAKE_THRESHOLD 0.67f
#define MALLOC_CAP_8BIT 1
#define DET_MODE_90 0
#define NVS_READONLY 0
#define NVS_READWRITE 1
#define pdTRUE 1
#define pdPASS 1
#define pdMS_TO_TICKS(x) (x)
#define ESP_LOGI(...) ((void)0)
#define ESP_LOGW(...) ((void)0)
typedef void *TaskHandle_t;
typedef int *SemaphoreHandle_t;
typedef int nvs_handle_t;
typedef struct {int unused;} model_iface_data_t;
typedef struct {const char *model_name[1];} srmodel_list_t;
typedef struct {
 model_iface_data_t *(*create)(const char *,int);
 void (*set_det_threshold)(model_iface_data_t *,float,int);
 int (*get_samp_chunksize)(model_iface_data_t *);
 int (*get_samp_rate)(model_iface_data_t *);
 int (*detect)(model_iface_data_t *,int16_t *);
 void (*destroy)(model_iface_data_t *);
} esp_wn_iface_t;
int64_t esp_timer_get_time(void);
size_t esp_get_free_heap_size(void);
size_t heap_caps_get_largest_free_block(int cap);
srmodel_list_t *srmodel_load(const void *data);
const esp_wn_iface_t *esp_wn_handle_from_name(const char *name);
void esp_srmodel_deinit(srmodel_list_t *list);
void badge_power_enter(void);
void badge_power_set_local_listening(bool value);
void badge_power_tick(bool busy,bool audio);
esp_err_t badge_power_leave(void);
esp_err_t bsp_audio_init(void);
esp_err_t bsp_audio_set_format(int rate,int bits,int channels);
esp_err_t bsp_audio_read(void *buffer,size_t bytes);
esp_err_t nvs_open(const char *,int,nvs_handle_t *);
esp_err_t nvs_get_u8(nvs_handle_t,const char *,uint8_t *);
esp_err_t nvs_set_u8(nvs_handle_t,const char *,uint8_t);
esp_err_t nvs_commit(nvs_handle_t);
void nvs_close(nvs_handle_t);
SemaphoreHandle_t xSemaphoreCreateBinary(void);
int xSemaphoreTake(SemaphoreHandle_t,unsigned);
void xSemaphoreGive(SemaphoreHandle_t);
void vSemaphoreDelete(SemaphoreHandle_t);
int xTaskCreate(void (*fn)(void *),const char *,unsigned,void *,unsigned,TaskHandle_t *);
void vTaskDelete(TaskHandle_t);
void vTaskSuspend(TaskHandle_t);
void taskYIELD(void);
