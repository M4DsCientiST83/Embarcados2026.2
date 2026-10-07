#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_task_wdt.h"
#include "esp_timer.h"

#define LED_PIN          GPIO_NUM_16
#define BUTTON_PIN       GPIO_NUM_17

#define DEBOUNCE_MS      50
#define LONG_PRESS_MS    2000
#define LED_TIMEOUT_MS   30000

static void configure_pins(void)
{
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);

    gpio_reset_pin(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_pulldown_en(BUTTON_PIN);
    gpio_pullup_dis(BUTTON_PIN);
}

void app_main(void) 
{
    vTaskPrioritySet(NULL, tskIDLE_PRIORITY);

    configure_pins();

    esp_task_wdt_add(NULL);

    bool led_on = false;
    uint32_t led_start_time = 0;

    int previous_reading = 0;
    int button_state = 0;

    uint32_t debounce_last_time = 0;
    uint32_t press_start_time = 0;
    bool long_press_handled = false;

    while (true) 
    {
        uint32_t time_now = (uint32_t)(esp_timer_get_time() / 1000);
        int reading = gpio_get_level(BUTTON_PIN);

        if (reading != previous_reading)
        {
            debounce_last_time = time_now;
            previous_reading = reading;
        }

        if ((time_now - debounce_last_time) >= DEBOUNCE_MS)
        {
            if (reading != button_state)
            {
                button_state = reading;

                if (button_state == 1)
                {
                    press_start_time = time_now;
                    long_press_handled = false;

                    if (!led_on)
                    {
                        led_on = true;
                        gpio_set_level(LED_PIN, 1);
                        led_start_time = time_now;
                    }
                    else
                    {
                        led_start_time = time_now;
                    }
                }
            }
        }

        if (button_state == 1 && !long_press_handled)
        {
            if ((time_now - press_start_time) >= LONG_PRESS_MS)
            {
                long_press_handled = true;

                if (led_on)
                {
                    led_on = false;
                    gpio_set_level(LED_PIN, 0);
                }
            }
        }

        if (led_on)
        {
            if ((time_now - led_start_time) >= LED_TIMEOUT_MS)
            {
                led_on = false;
                gpio_set_level(LED_PIN, 0);
            }
        }

        esp_task_wdt_reset();
        taskYIELD();
    }
}