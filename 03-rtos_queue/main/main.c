#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_random.h"
#include "freertos/queue.h"

static const char* TAG = "main";

void task_1(void* pvParameter)
{
    static uint16_t time = 0;
    while(1)
    {
        /**
         * 每随机10~1000ms产生一次数据
         */
        time = esp_random() % 1000 + 10;
        if(time > 1000) time = 1000;

        xQueueSend((QueueHandle_t)pvParameter, &time, 0);
        ESP_LOGI(TAG, "task 1 will delay %d ms", time);

        vTaskDelay(pdMS_TO_TICKS(time));
        
    }
}

void task_2(void* pvParameter)
{
    static uint16_t time = 0;
    while(1)
    {
        /**
         * 打印任务一延迟的时间/数据
         */
        if(xQueueReceive((QueueHandle_t)pvParameter, &time, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(TAG, "task 2 Recv task 1 delay %d ms", time);
        }
    }
}
QueueHandle_t xQueue = {0};
void app_main(void)
{
    xQueue = xQueueCreate(10, sizeof(uint16_t));
    xTaskCreatePinnedToCore(task_1, "task 1", 2048, xQueue, 1, NULL, 1);
    xTaskCreatePinnedToCore(task_2, "task 2", 2048, xQueue, 1, NULL, 1);
}
