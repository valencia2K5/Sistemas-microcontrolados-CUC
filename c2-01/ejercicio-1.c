/*
Integrantes:
- José Valencia
- Jesús Ramírez
- Daniel Ibañez

Enunciado:
[Ejercicio 1 del taller del ESP32]
*/


#include <stdio.h>              // Para funciones de entrada/salida estándar como printf
#include <string.h>             // Para funciones de manejo de cadenas (strlen, strcmp, strtok)
#include <stdlib.h>             // Para atoi y otras funciones útiles
#include "freertos/FreeRTOS.h"  // Para usar FreeRTOS (sistema operativo del ESP32)
#include "freertos/task.h"      // Para crear tareas y usar delays
#include "driver/uart.h"        // Para configurar y usar el puerto UART (comunicación serial)

// Tamaño del buffer de entrada serial
#define BUF_SIZE 1024

// Variables globales para estadísticas
int minimo = 100;       // Valor mínimo ingresado (inicializado en 100 para que cualquier número menor lo reemplace)
int maximo = -1;        // Valor máximo ingresado (inicializado en -1 para que cualquier número mayor lo reemplace)
int ultimo = 0;         // Último número válido ingresado
int suma = 0;           // Suma total de los números válidos
int cantidad = 0;       // Cantidad de números válidos ingresados

// Función principal del programa en ESP-IDF
void app_main(void) {
    const uart_port_t uart_num = UART_NUM_0; // Selecciona UART0 (la terminal por defecto)

    // Configura los parámetros de la comunicación UART
    uart_config_t uart_config = {
        .baud_rate = 115200,                      // Velocidad de transmisión (baudios)
        .data_bits = UART_DATA_8_BITS,            // 8 bits de datos por byte
        .parity    = UART_PARITY_DISABLE,         // Sin bit de paridad
        .stop_bits = UART_STOP_BITS_1,            // 1 bit de parada
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE     // Sin control de flujo
    };

    // Instala el driver UART con un buffer de recepción
    uart_driver_install(uart_num, BUF_SIZE, 0, 0, NULL, 0);

    // Aplica la configuración UART
    uart_param_config(uart_num, &uart_config);

    // No cambiamos los pines, usamos los predeterminados
    uart_set_pin(uart_num, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Mensaje inicial por consola
    printf("Esperando datos entre 0 y 99...\n");

    // Buffer donde se guardará la entrada serial
    uint8_t data[BUF_SIZE];

    // Bucle infinito (típico en microcontroladores)
    while (1) {
        // Leer bytes desde UART (con timeout de 1 segundo)
        int len = uart_read_bytes(uart_num, data, BUF_SIZE - 1, pdMS_TO_TICKS(1000));

        if (len > 0) {
            data[len] = '\0'; // Agrega terminador nulo para que sea una cadena válida

            // Extrae la primera línea (remueve saltos de línea y retorno de carro)
            char *entrada = strtok((char *)data, "\r\n");

            // Si la entrada es vacía, no hacer nada
            if (entrada == NULL || strlen(entrada) == 0) continue;

            // Convierte la entrada a entero
            int numero = atoi(entrada);

            // Convierte el número de vuelta a string para validar que fue un número limpio
            char numero_str[16];
            sprintf(numero_str, "%d", numero);

            // Validación completa: debe estar entre 0 y 99, y debe coincidir exactamente con la entrada
            if (numero >= 0 && numero <= 99 && strcmp(numero_str, entrada) == 0) {
                // Si es válido, actualizar estadísticas
                ultimo = numero;
                if (numero < minimo) minimo = numero;
                if (numero > maximo) maximo = numero;
                suma += numero;
                cantidad++;

                // Calcular promedio
                float promedio = (float)suma / cantidad;

                // Imprimir resultados
                printf("Último: %d. Mínimo: %d. Máximo: %d. Promedio: %.2f\n",
                       ultimo, minimo, maximo, promedio);
            } else {
                // Si la entrada no es válida, mostrar error
                printf("Número inválido. Ingrese un número entre 0 y 99.\n");
            }
        }

        // Esperar 100 ms antes de volver a leer
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
