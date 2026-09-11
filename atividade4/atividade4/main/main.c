#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LED1 GPIO_NUM_4
#define LED2 GPIO_NUM_16
#define LED3 GPIO_NUM_17
#define LED4 GPIO_NUM_18

#define WHITE_BUTTON GPIO_NUM_21
#define BLUE_BUTTON  GPIO_NUM_19

static uint8_t counter = 0;
static uint8_t step = 1;

void gpio_init(void)
{
    gpio_reset_pin(LED1);
    gpio_set_direction(LED1, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LED2);
    gpio_set_direction(LED2, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LED3);
    gpio_set_direction(LED3, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LED4);
    gpio_set_direction(LED4, GPIO_MODE_OUTPUT);

    gpio_reset_pin(WHITE_BUTTON);
    gpio_set_direction(WHITE_BUTTON, GPIO_MODE_INPUT);
    gpio_set_pull_mode(WHITE_BUTTON, GPIO_PULLDOWN_ONLY);

    gpio_reset_pin(BLUE_BUTTON);
    gpio_set_direction(BLUE_BUTTON, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BLUE_BUTTON, GPIO_PULLDOWN_ONLY);
}

void update_leds(uint8_t val)
{
    gpio_set_level(LED1, (val >> 0) & 1);
    gpio_set_level(LED2, (val >> 1) & 1);
    gpio_set_level(LED3, (val >> 2) & 1);
    gpio_set_level(LED4, (val >> 3) & 1);
}

void app_main(void)
{
    gpio_init();
    update_leds(counter);

    int last_white = 0;
    int last_blue = 0;

    while (1) 
    {
        int current_white = gpio_get_level(WHITE_BUTTON);
        int current_blue = gpio_get_level(BLUE_BUTTON);

        if (current_white == 1 && last_white == 0) 
        {
            counter = (counter + step) % 16;
            update_leds(counter);
        }
        last_white = current_white;

        if (current_blue == 1 && last_blue == 0) 
        {
            step = (step == 1) ? 2 : 1;
        }
        last_blue = current_blue;

        vTaskDelay(pdMS_TO_TICKS(50)); 
    }
}