#pragma once
typedef void *TaskHandle_t;
TaskHandle_t xTaskGetCurrentTaskHandle(void);
void vTaskSuspend(TaskHandle_t);
void vTaskResume(TaskHandle_t);
void vTaskDelay(unsigned);
