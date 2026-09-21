#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_timer.h"

#define LED1 GPIO_NUM_18
#define LED2 GPIO_NUM_17
#define LED3 GPIO_NUM_16
#define LED4 GPIO_NUM_4

#define BTN_A GPIO_NUM_21
#define BTN_B GPIO_NUM_19

#define PWM_LED    GPIO_NUM_23
#define PWM_BUZZER GPIO_NUM_22

#define LEDC_TIMER          LEDC_TIMER_0
#define LEDC_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL_LED    LEDC_CHANNEL_0
#define LEDC_CHANNEL_BUZZER LEDC_CHANNEL_1
#define LEDC_DUTY_RES       LEDC_TIMER_13_BIT
#define LEDC_FREQUENCY      1000

volatile uint8_t counter = 0;
volatile bool update_flag = true;

volatile uint64_t last_time_a = 0;
volatile uint64_t last_time_b = 0;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    uint64_t current_time = esp_timer_get_time();

    if (gpio_num == BTN_A && (current_time - last_time_a > 200000)) 
    {
        counter = (counter + 1) & 0x0F;
        update_flag = true;
        last_time_a = current_time;
    } 
    else if (gpio_num == BTN_B && (current_time - last_time_b > 200000)) 
    {
        counter = (counter - 1) & 0x0F;
        update_flag = true;
        last_time_b = current_time;
    }
}

void hw_init(void)
{
    gpio_reset_pin(LED1);
    gpio_set_direction(LED1, GPIO_MODE_OUTPUT);
    gpio_reset_pin(LED2);
    gpio_set_direction(LED2, GPIO_MODE_OUTPUT);
    gpio_reset_pin(LED3);
    gpio_set_direction(LED3, GPIO_MODE_OUTPUT);
    gpio_reset_pin(LED4);
    gpio_set_direction(LED4, GPIO_MODE_OUTPUT);

    gpio_reset_pin(BTN_A);
    gpio_set_direction(BTN_A, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BTN_A, GPIO_PULLDOWN_ONLY);
    gpio_set_intr_type(BTN_A, GPIO_INTR_POSEDGE);

    gpio_reset_pin(BTN_B);
    gpio_set_direction(BTN_B, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BTN_B, GPIO_PULLDOWN_ONLY);
    gpio_set_intr_type(BTN_B, GPIO_INTR_POSEDGE);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BTN_A, gpio_isr_handler, (void*) BTN_A);
    gpio_isr_handler_add(BTN_B, gpio_isr_handler, (void*) BTN_B);

    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel_led = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_LED,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = PWM_LED,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel_led);

    ledc_channel_config_t ledc_channel_buzzer = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_BUZZER,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = PWM_BUZZER,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel_buzzer);
}

void update_hardware()
{
    gpio_set_level(LED1, (counter >> 0) & 1);
    gpio_set_level(LED2, (counter >> 1) & 1);
    gpio_set_level(LED3, (counter >> 2) & 1);
    gpio_set_level(LED4, (counter >> 3) & 1);

    uint32_t duty = (counter * 8191) / 15;
    
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_LED, duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_LED);
    
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_BUZZER, duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_BUZZER);
}

void app_main(void)
{
    hw_init();

    while (1) 
    {
        if (update_flag) 
        {
            update_hardware();
            update_flag = false;
			printf("Counter: %d | Duty Cycle: %u / 8191\n", counter, (unsigned int)((counter * 8191) / 15));
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}