#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_attr.h"

#define LED_PIN          GPIO_NUM_17
#define BUTTON_PIN       GPIO_NUM_16

#define DEBOUNCE_US      (50 * 1000)
#define LONG_PRESS_US    (2000 * 1000)
#define LED_TIMEOUT_US   (10000 * 1000)

static esp_timer_handle_t led_timer = NULL;
static esp_timer_handle_t long_press_timer = NULL;
static esp_timer_handle_t debounce_timer = NULL;
static QueueHandle_t button_evt_queue = NULL;

static void led_timer_callback(void* arg)
{
    gpio_set_level(LED_PIN, 0);
}

static void long_press_timer_callback(void* arg)
{
    gpio_set_level(LED_PIN, 0);
    esp_timer_stop(led_timer);
}

static void debounce_timer_callback(void* arg)
{
    int state = gpio_get_level(BUTTON_PIN);

    if (state == 1) 
    {
        gpio_set_level(LED_PIN, 1);

        esp_timer_stop(led_timer);
        esp_timer_start_once(led_timer, LED_TIMEOUT_US);

        esp_timer_stop(long_press_timer);
        esp_timer_start_once(long_press_timer, LONG_PRESS_US);
    } 
    else 
    {
        esp_timer_stop(long_press_timer);
    }
}

static void IRAM_ATTR button_isr_handler(void* arg)
{
    uint8_t dummy = 1;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(button_evt_queue, &dummy, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken) 
    {
        portYIELD_FROM_ISR();
    }
}

static void button_task(void* arg)
{
    uint8_t dummy;

    while (true) 
    {
        if (xQueueReceive(button_evt_queue, &dummy, portMAX_DELAY) == pdTRUE) 
        {
            esp_timer_stop(debounce_timer);
            esp_timer_start_once(debounce_timer, DEBOUNCE_US);
        }
    }
}

static void configure_pins(void)
{
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);

    gpio_reset_pin(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_pulldown_en(BUTTON_PIN);
    gpio_pullup_dis(BUTTON_PIN);

    gpio_set_intr_type(BUTTON_PIN, GPIO_INTR_ANYEDGE);
}

static void configure_timers(void)
{
    const esp_timer_create_args_t led_timer_args = {
        .callback = &led_timer_callback,
        .name = "led_timer"
    };
    esp_timer_create(&led_timer_args, &led_timer);

    const esp_timer_create_args_t long_press_args = {
        .callback = &long_press_timer_callback,
        .name = "long_press_timer"
    };
    esp_timer_create(&long_press_args, &long_press_timer);

    const esp_timer_create_args_t debounce_args = {
        .callback = &debounce_timer_callback,
        .name = "debounce_timer"
    };
    esp_timer_create(&debounce_args, &debounce_timer);
}

void app_main(void)
{
    button_evt_queue = xQueueCreate(20, sizeof(uint8_t));

    configure_pins();
    configure_timers();

    xTaskCreate(button_task, "button_task", 2048, NULL, 10, NULL);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);

    vTaskDelete(NULL);
}