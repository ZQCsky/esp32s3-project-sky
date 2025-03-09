#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_random.h"
#include "freertos/event_groups.h"

/**
 * 事件组
 */
EventGroupHandle_t event_group;
/**
 * 事件组中事件的bit位
 */
#define EV_BIT_0 (1<<0)

/**
 * 直达任务通知
 */
TaskHandle_t task1_handle = NULL;
TaskHandle_t task2_handle = NULL;
void task_1(void* pvParameter)
{
    uint16_t time = 0;
    while(1)
    {
        /**
         * 每10~1000ms随机一个数字，
         * 然后设置事件组中的bit位EV_BIT_0
         */
        time = esp_random() % 1000 + 10;
        xEventGroupSetBits(event_group, EV_BIT_0);
        ESP_LOGI("Task1", "task_1 set EventGroup bits 0x%x", EV_BIT_0);
        vTaskDelay(pdMS_TO_TICKS(time));
        /**
         * 延时到发送通知。
         */
        xTaskNotify(task2_handle, time, eSetValueWithOverwrite);
    }
}

void task_2(void* pvParameter)
{
    EventBits_t Bit = 0;
    uint32_t notify_value = 0;
    while(1)
    {
        /**
         * 等待事件组中的bit位EV_BIT_0被设置
         */
        Bit = xEventGroupWaitBits(event_group, EV_BIT_0, pdTRUE, pdFALSE, 10);
        if(EV_BIT_0 & Bit)
            ESP_LOGI("Task2", "task_2 get EventGroup bits 0x%x", EV_BIT_0);

        xTaskNotifyWait(0, ULONG_MAX, &notify_value, 10);
        if(notify_value > 0)
        {
            ESP_LOGI("Task2", "task_2 get notify value %ld", notify_value);
            notify_value = 0;
        }
            
    }
}
void app_main(void)
{
    event_group = xEventGroupCreate();
    xTaskCreatePinnedToCore(task_1, "task 1", 4096, NULL, 1, &task1_handle, 1);
    xTaskCreatePinnedToCore(task_2, "task 2", 4096, NULL, 1, &task2_handle, 1);
}
