#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char* TAG = "main";

void task_1(void* pvParameter)
{
    while(1)
    {
        ESP_LOGI(TAG, "task 1");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void task_2(void* pvParameter)
{
    while(1)
    {
        ESP_LOGI(TAG, "task 2");
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
void app_main(void)
{
    xTaskCreatePinnedToCore(task_1, "task 1", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(task_2, "task 2", 2048, NULL, 1, NULL, 1);
}
