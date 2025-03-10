#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "freertos/event_groups.h"
TaskHandle_t task1_handle = NULL;

/**
 * 源地的板子没有LED，使用GPIO1
 */
#define LED_GPIO_PIN GPIO_NUM_1
#define LEDC_EV_FINISH_FULL         (1<<0)
#define LEDC_EV_FINISH_EMPTY        (1<<1)

bool IRAM_ATTR ledc_finish_cb(const ledc_cb_param_t *param, void *user_arg)
{
    BaseType_t pxHigherPriorityTaskWoken= 0;
    if(param->duty == 0)
        xTaskNotifyFromISR(task1_handle,param->duty,eSetValueWithOverwrite,&pxHigherPriorityTaskWoken);
    else
        xTaskNotifyFromISR(task1_handle,param->duty,eSetValueWithOverwrite,&pxHigherPriorityTaskWoken);
    return pxHigherPriorityTaskWoken;
}
ledc_cbs_t cbs = {
    .fade_cb = ledc_finish_cb, // 渐变完成回调函数
};
void task_1(void* pvParameter)
{
    //bool led_state = 0;
    uint32_t notify_value = 0;
    while(1)
    {
        // gpio_set_level(LED_GPIO_PIN, led_state);
        // led_state = !led_state;
        // vTaskDelay(pdMS_TO_TICKS(500));
        xTaskNotifyWait(0, ULONG_MAX, &notify_value,portMAX_DELAY);
        if(notify_value != 0)
        {
            ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0, 0, 2000); // 2000ms
            ledc_fade_start(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);
            ESP_LOGI("Task1", "task_1 get notify value %ld", notify_value);
            notify_value = 0;
        }
        else
        {
            ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0, 8191, 2000); // 2000ms
            ledc_fade_start(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);
            ESP_LOGI("Task1", "task_1 get notify value %ld", notify_value);
            notify_value = 0;
        }
        ledc_cb_register(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, &cbs, NULL);
    }
}

static void ledc_init(void)
{
    // 初始化GPIO
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << LED_GPIO_PIN,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = LEDC_TIMER_0,      //定时器通道0 
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK, 
    };  
    ledc_timer_config(&ledc_timer);
    ledc_channel_config_t ledc_channel = {
       .gpio_num = LED_GPIO_PIN,
       .speed_mode = LEDC_LOW_SPEED_MODE,
       .channel = LEDC_CHANNEL_0,
       .intr_type = LEDC_INTR_DISABLE,
       .timer_sel = LEDC_TIMER_0,
       .duty = 0,
    };
    ledc_channel_config(&ledc_channel);

    ledc_fade_func_install(0); // 开启PWM模块
    ledc_set_fade_with_time(ledc_channel.speed_mode, ledc_channel.channel, 8191, 2000); // 2000ms
    ledc_fade_start(ledc_channel.speed_mode, ledc_channel.channel, LEDC_FADE_NO_WAIT);

    ledc_cb_register(ledc_channel.speed_mode, ledc_channel.channel, &cbs, NULL);
}
void app_main(void)
{
    // 外设初始化
    ledc_init();
    // 创建任务
    xTaskCreatePinnedToCore(task_1, "task 1", 4096, NULL, 1, &task1_handle, 1);
}
