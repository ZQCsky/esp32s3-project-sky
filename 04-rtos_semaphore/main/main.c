#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_random.h"
#include "freertos/semphr.h"


static const char* TAG = "main";
/**
 * 同一个资源可以给一个(二值信号量)或多个(计数信号量)消费者使用。
 * Give:临界区资源可访问数加一
 * Take:临界区资源可访问数减一
 * 使用时应先锁定资源：Take成功，则资源可以访问。
 * 使用完成后释放资源：Give,表示解除资源占用，其他访问者可以访问/使用。
 */
static SemaphoreHandle_t semap = {0};
/**
 * 互斥锁
 * 访问临界区资源前，获取资源使用权，释放前其他任务不可使用。
 */
static SemaphoreHandle_t mutex = {0};
/**
 * 临界区资源
 */
uint32_t BufferCount = 0;

void task_1(void* pvParameter)
{
    static uint16_t time = 0;
    while(1)
    {
        /**
         * 每随机10~1000ms产生一次数据,并释放信号量
         */
        time = esp_random() % 1000 + 10;
        if(time > 1000) time = 1000;

        xSemaphoreGive((SemaphoreHandle_t)pvParameter);
        ESP_LOGI("Task 1","in app_main the min free stack size is %ld", (int32_t)uxTaskGetStackHighWaterMark(NULL));
        ESP_LOGI("Task 1", "task 1 will delay %d ms", time);

        if(xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) // 获取临界区资源使用权
        {
            BufferCount += time;
            ESP_LOGI(TAG, "task 1 BuffCount = %ld", BufferCount);
            xSemaphoreGive(mutex);// 释放临界区资源使用权
        }
        vTaskDelay(pdMS_TO_TICKS(time));
        
    }
}

void task_2(void* pvParameter)
{
    while(1)
    {
        /**
         * 打印任务一延迟的时间/数据
         */
        if(xSemaphoreTake((QueueHandle_t)pvParameter, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI("Task 2","in app_main the min free stack size is %ld", (int32_t)uxTaskGetStackHighWaterMark(NULL));
            ESP_LOGI("Task 2", "task 2 Recv task 1 BuffFree");
            if(xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) // 获取临界区资源使用权
            {
                BufferCount /= 2;
                ESP_LOGI(TAG, "task 1 BuffCount = %ld", BufferCount);
                xSemaphoreGive(mutex);// 释放临界区资源使用权
            }
        }
    }
}
void app_main(void)
{
    semap = xSemaphoreCreateCounting(10,5);
    mutex = xSemaphoreCreateMutex();
    xTaskCreatePinnedToCore(task_1, "task 1", 4096, semap, 1, NULL, 1);
    xTaskCreatePinnedToCore(task_2, "task 2", 4096, semap, 1, NULL, 1);
}
