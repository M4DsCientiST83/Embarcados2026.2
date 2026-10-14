#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#define UART_PORT       UART_NUM_2
#define TXD_PIN         GPIO_NUM_17
#define RXD_PIN         GPIO_NUM_16
#define LED_PIN         GPIO_NUM_19

#define BUF_SIZE        1024

void init_uart2(void) 
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0));
}

void init_gpio(void) 
{
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);
}

void app_main(void) 
{
    init_gpio();
    init_uart2();

    uint8_t rx_buffer[BUF_SIZE];
    bool alternar_comando = true;

    printf("Sistema iniciado! Monitorando a UART2...\n\n");

    while (1) 
	{
        const char *comando_envio = alternar_comando ? "LIGAR" : "DESLIGAR";

        uart_flush_input(UART_PORT);

        printf("[TX] Enviando para UART2: %s\n", comando_envio);
        
        uart_write_bytes(UART_PORT, comando_envio, strlen(comando_envio));

        int len = uart_read_bytes(UART_PORT, rx_buffer, BUF_SIZE - 1, pdMS_TO_TICKS(1000));

        if (len > 0) 
		{
            rx_buffer[len] = '\0';
            printf("[RX] Recebido da UART2: %s\n", (char *)rx_buffer);

            if (strcmp((char *)rx_buffer, "LIGAR") == 0) 
			{
                gpio_set_level(LED_PIN, 1);
                printf("--> Ação: LED LIGADO\n\n");
            } 
            else if (strcmp((char *)rx_buffer, "DESLIGAR") == 0) 
			{
                gpio_set_level(LED_PIN, 0);
                printf("--> Ação: LED DESLIGADO\n\n");
            } 
            else 
			{
                printf("--> Ação: Dado corrompido recebido!\n\n");
            }
        }

        alternar_comando = !alternar_comando;
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}