#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define GREEN_LED GPIO_NUM_22
#define YELLOW_LED GPIO_NUM_23
#define RED_LED GPIO_NUM_5
#define BLUE_LED GPIO_NUM_4

uint8_t value;

  void led_config(void)
  {
    gpio_reset_pin(GREEN_LED);
    gpio_set_direction(GREEN_LED, GPIO_MODE_OUTPUT);

    gpio_reset_pin(YELLOW_LED);
    gpio_set_direction(YELLOW_LED, GPIO_MODE_OUTPUT);
    
    gpio_reset_pin(RED_LED);
    gpio_set_direction(RED_LED, GPIO_MODE_OUTPUT);

    gpio_reset_pin(BLUE_LED);
    gpio_set_direction(BLUE_LED, GPIO_MODE_OUTPUT);
  }

  void fase1(void)
  {
    for (int i = 0; i < 16; i++)
    {
      gpio_set_level(GREEN_LED, (value >> 0) & 1);
      gpio_set_level(YELLOW_LED, (value >> 1) & 1);
      gpio_set_level(RED_LED, (value >> 2) & 1);
      gpio_set_level(BLUE_LED, (value >> 3) & 1);
      value = (value + 1) % 16;
      vTaskDelay(pdMS_TO_TICKS(500));
    }
  }

  void fase2(void)
  {
    gpio_set_level(GREEN_LED, 0);
    gpio_set_level(YELLOW_LED, 0);
    gpio_set_level(RED_LED, 0);
    gpio_set_level(BLUE_LED, 0);


    gpio_set_level(GREEN_LED, 1);
    vTaskDelay(pdMS_TO_TICKS(500));

    gpio_set_level(GREEN_LED, 0);
    gpio_set_level(YELLOW_LED, 1);
    vTaskDelay(pdMS_TO_TICKS(500));

    gpio_set_level(YELLOW_LED, 0);
    gpio_set_level(RED_LED, 1);
    vTaskDelay(pdMS_TO_TICKS(500));

    gpio_set_level(RED_LED, 0);
    gpio_set_level(BLUE_LED, 1);
    vTaskDelay(pdMS_TO_TICKS(500));

    gpio_set_level(BLUE_LED, 0);
    vTaskDelay(pdMS_TO_TICKS(500));


    gpio_set_level(BLUE_LED, 1);
    vTaskDelay(pdMS_TO_TICKS(500));

    gpio_set_level(BLUE_LED, 0);
    gpio_set_level(RED_LED, 1);
    vTaskDelay(pdMS_TO_TICKS(500));

    gpio_set_level(RED_LED, 0);
    gpio_set_level(YELLOW_LED, 1);
    vTaskDelay(pdMS_TO_TICKS(500));

    gpio_set_level(YELLOW_LED, 0);
    gpio_set_level(GREEN_LED, 1);
    vTaskDelay(pdMS_TO_TICKS(500));

    gpio_set_level(GREEN_LED, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
  }

void app_main() 
{ 
  led_config();
  
  while (true) 
  {
    fase1();
    fase2();
  }
}
