/*
Integrantes:
- José Valencia
- Jesús Ramírez
- Daniel Ibañez

Enunciado:
[Ejercicio 2 del taller de ESP32]
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"

#define UART_PORT UART_NUM_0
#define BUF_SIZE 128

// Verifica si el texto ingresado es un número entero
bool is_integer(const char *str) {
    if (str == NULL || *str == '\0') return false;
    if (*str == '-' || *str == '+') str++;
    if (*str == '\0') return false;
    while (*str) {
        if (!isdigit((unsigned char)*str)) return false;
        str++;
    }
    return true;
}

// Calcula el cuadrado sumando los N primeros impares
int square_by_odds(int n) {
    int sum = 0, odd = 1;
    for (int i = 0; i < abs(n); i++) {
        sum += odd;
        odd += 2;
    }
    return sum;
}

void app_main(void) {
    // Configurar UART
    uart_config_t config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_PORT, &config);
    uart_set_pin(UART_PORT, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    char buffer[BUF_SIZE];

    const char *msg = "Ingresa un número entero para calcular su cuadrado:\n";
    uart_write_bytes(UART_PORT, msg, strlen(msg));

    while (1) {
        memset(buffer, 0, BUF_SIZE);
        int len = uart_read_bytes(UART_PORT, (uint8_t*)buffer, BUF_SIZE - 1, pdMS_TO_TICKS(500));

        if (len > 0) {
            buffer[len] = '\0';
            buffer[strcspn(buffer, "\r\n")] = '\0';  // Elimina salto de línea

            if (is_integer(buffer)) {
                int n = atoi(buffer);
                int result = square_by_odds(n);
                char response[64];
                snprintf(response, sizeof(response), "El cuadrado de %d es %d\n", n, result);
                uart_write_bytes(UART_PORT, response, strlen(response));
            } else {
                const char *err = "Entrada inválida. Intenta con un número entero.\n";
                uart_write_bytes(UART_PORT, err, strlen(err));
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}