/*
Integrantes:
- José Valencia
- Jesús Ramírez
- Daniel Ibañez

Enunciado:
[Ejercicio 2 del taller de ESP32]
*/


#include <stdio.h>      // Librería estándar para entrada/salida
#include <stdlib.h>     // Librería para funciones como atoi(), abs()
#include <string.h>     // Para manejo de strings como strcpy, strcat, etc.
#include <ctype.h>      // Para funciones de validación de caracteres como isdigit()

// Librerías de ESP-IDF
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"

// Definiciones de configuración de UART
#define UART_PORT UART_NUM_0     // Usamos el puerto UART0
#define BUF_SIZE 128             // Tamaño del buffer de recepción

// -----------------------------------------------------------------------------
// Función que verifica si una cadena representa un número entero válido
// -----------------------------------------------------------------------------
bool is_integer(const char *str) {
    if (str == NULL || *str == '\0') return false;  // Verifica cadena vacía
    if (*str == '-' || *str == '+') str++;          // Permite signo negativo o positivo
    if (*str == '\0') return false;                 // Si luego del signo no hay nada, es inválido

    while (*str) {
        if (!isdigit((unsigned char)*str)) return false;  // Verifica que cada carácter sea dígito
        str++;
    }

    return true;  // Si pasó todas las validaciones, es un número entero
}

// -----------------------------------------------------------------------------
// Función que calcula el cuadrado de un número N sumando los N primeros impares
// También genera una cadena de texto con la serie, por ejemplo: "1+3+5+...=N*N"
// -----------------------------------------------------------------------------
int square_with_series(int n, char *series, size_t size) {
    int sum = 0;
    int odd = 1;  // El primer número impar es 1
    int count = abs(n);  // Usamos valor absoluto por si el usuario ingresa negativo
    char temp[16];       // Buffer temporal para ir armando la serie

    series[0] = '\0';    // Inicializa la cadena vacía

    for (int i = 0; i < count; i++) {
        sum += odd;      // Suma el número impar actual

        // Convierte el impar actual a texto y lo agrega a la serie
        snprintf(temp, sizeof(temp), "%d", odd);
        strncat(series, temp, size - strlen(series) - 1);

        // Agrega el signo '+' si no es el último número
        if (i < count - 1) {
            strncat(series, "+", size - strlen(series) - 1);
        }

        odd += 2;  // El siguiente impar es 2 unidades mayor
    }

    return sum;  // Devuelve el resultado de la suma (cuadrado de N)
}

// -----------------------------------------------------------------------------
// Función principal del programa (punto de entrada en ESP-IDF)
// -----------------------------------------------------------------------------
void app_main(void) {
    // Configuración de parámetros UART
    uart_config_t config = {
        .baud_rate = 115200,                      // Velocidad de comunicación
        .data_bits = UART_DATA_8_BITS,            // 8 bits de datos
        .parity    = UART_PARITY_DISABLE,         // Sin bit de paridad
        .stop_bits = UART_STOP_BITS_1,            // 1 bit de parada
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE     // Sin control de flujo
    };

    // Inicializa el controlador UART
    uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_PORT, &config);

    // Establece los pines UART (sin cambios, ya que usamos UART0)
    uart_set_pin(UART_PORT, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    char buffer[BUF_SIZE];       // Buffer para almacenar lo leído por UART
    char series[256];            // Para almacenar la serie de impares

    // Mensaje inicial que se envía al usuario
    const char *msg = "Ingresa un número entero para calcular su cuadrado como suma de impares:\n";
    uart_write_bytes(UART_PORT, msg, strlen(msg));

    // Bucle principal
    while (1) {
        memset(buffer, 0, BUF_SIZE);  // Limpia el buffer antes de recibir

        // Lee datos desde UART
        int len = uart_read_bytes(UART_PORT, (uint8_t*)buffer, BUF_SIZE - 1, pdMS_TO_TICKS(500));

        if (len > 0) {
            buffer[len] = '\0';  // Asegura el fin de cadena

            // Elimina caracteres de nueva línea (\n o \r)
            buffer[strcspn(buffer, "\r\n")] = '\0';

            // Verifica si la entrada es un número entero válido
            if (is_integer(buffer)) {
                int n = atoi(buffer);  // Convierte string a entero

                // Calcula el cuadrado usando suma de impares y genera serie
                int result = square_with_series(n, series, sizeof(series));

                // Prepara el mensaje de salida con formato
                char response[512];
                snprintf(response, sizeof(response),
                         "Cuadrado de %d = %s = %d\n", n, series, result);

                // Envía el resultado por UART
                uart_write_bytes(UART_PORT, response, strlen(response));
            } else {
                // Mensaje de error si la entrada no es un número entero
                const char *err = "Entrada inválida. Por favor ingresa un número entero.\n";
                uart_write_bytes(UART_PORT, err, strlen(err));
            }
        }

        // Espera pequeña antes de repetir
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
