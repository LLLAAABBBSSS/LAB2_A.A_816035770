#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_sleep.h"
#include "logging_stack.h"
#include "logging_levels.h"

#define LED_PIN 2                                       // GPIO led pin

#ifndef LIBRARY_LOG_NAME                                //from https://www.freertos.org/Documentation/03-Libraries/06-Logging
    #define LIBRARY_LOG_NAME "LAB2"
#endif 

#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL LOG_ERROR
#endif

#if !defined( LOG_METADATA_FORMAT ) && !defined( LOG_METADATA_ARGS )
    #define LOG_METADATA_FORMAT "[file:line]"
    #define LOG_METADATA_ARGS _FILE_, _LINE_
#endif

#ifndef MyLogger
  #define MyLogger(fmt, ...)  ESP_LOGI(LIBRARY_LOG_NAME, fmt, ##__VA_ARGS__)
#endif
s
#ifndef SdkLog
    #define SdkLog( message ) MyLogger message 
#endif 

void math_sleep()                                       //math "sleep" function 
{
    TickType_t start = xTaskGetTickCount();
    const TickType_t duration = pdMS_TO_TICKS(500);     //makes 500ms so 0.5s

    volatile uint32_t i = 2;                        //volatile is used to ensure that the function is not skipped
    volatile uint32_t j = 3;                        //u is for unsigned    
    //volatile uint32_t k = 0u;

    while ((xTaskGetTickCount()-start)<duration)
    {
        i = i+j;                                    //addition 
        //j = j^i;                                        //XOR 
        //k = k + 67u;                                    //addition

    }
}

const int LED_ON_TASK_PRIORITY = 3;                    //task priorities
const int LED_OFF_TASK_PRIORITY = 2;
const int PRINT_TASK_PRIORITY = 1;

SemaphoreHandle_t xMutex;

void led_on_task(void *pvParameters)
{
    const char *TAG = "LED_ON_TASK";
    while (1)
    {
        if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
        {
            gpio_set_level(LED_PIN, 1);
            math_sleep();                              //math sleep function 
            LogMessage("LED ON");
            ESP_LOGI(TAG, "LED ON");
            xSemaphoreGive(xMutex);
        }
        taskYIELD();
    }
}

void led_off_task(void *pvParameters)
{
    const char *TAG = "LED_OFF_TASK";
    while (1)
    {
        if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
        {
            gpio_set_level(LED_PIN, 0);
            LogMessage("LED OFF");
            ESP_LOGI(TAG, "LED OFF");
            xSemaphoreGive(xMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void print_task(void *pvParameters)
{
    const char *TAG = "STATUS_TASK";
    while (1)
    {
        printf("Status Message\n");
        LogMessage("STATUS MESSAGE");
        ESP_LOGI(TAG, "STATUS MESSAGE");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    LogInit();
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << LED_PIN);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    xMutex = xSemaphoreCreateMutex();
    if (xMutex == NULL)
    {
        printf("Mutex creation failed!\n");
        return;
    }

    xTaskCreate(led_on_task, "LED_ON_TASK", 2048, NULL, LED_ON_TASK_PRIORITY, NULL);
    xTaskCreate(led_off_task, "LED_OFF_TASK", 2048, NULL, LED_OFF_TASK_PRIORITY, NULL);
    xTaskCreate(print_task, "PRINT_TASK", 2048, NULL, PRINT_TASK_PRIORITY, NULL);

    printf("Tasks started successfully.\n");
}
