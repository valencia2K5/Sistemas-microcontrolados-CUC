/*
Integrantes:
- José Valencia
- Jesús Ramírez
- Daniel Ibañez

Enunciado:
[Ejercicio 3 del taller del ESP32]
*/


#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/touch_pad.h"     // Manejo de pines táctiles
#include "esp_log.h"              // Para imprimir mensajes por consola
#include "esp_timer.h"            // Para obtener tiempos de duración de los toques

// Pines táctiles definidos
#define TOUCH_PIN      TOUCH_PAD_NUM0  // Pin táctil principal para ingresar la secuencia (GPIO 4)
#define VALIDAR_PIN    TOUCH_PAD_NUM3  // Pin táctil secundario para validar la secuencia (GPIO 15)

// Parámetros para definir el tiempo de toque corto y largo (en milisegundos)
#define TOQUE_CORTO_MS 1000
#define TOQUE_LARGO_MS 3000

// Umbral de lectura que indica si se ha tocado el pin táctil
#define UMBRAL_TOQUE   120

// Tag para los logs de ESP-IDF
static const char *TAG = "TOQUE";

// Secuencia esperada: {toques largos, toques cortos, toques largos}
int secuenciaEsperada[] = {3, 1, 3}; // 3 -> largo, 1 -> corto
int estado = 0;                      // Indica en qué parte de la secuencia estamos (0 a 2)
int conteoToques = 0;                // Cuenta cuántos toques válidos se han hecho en el estado actual

// Variables para manejar detección de toques
bool tocando = false;
int64_t tiempoInicio = 0;

// -----------------------------------------------------------------------------
// Reinicia la secuencia al estado inicial
// -----------------------------------------------------------------------------
void resetearSecuencia() {
    estado = 0;
    conteoToques = 0;
}

// -----------------------------------------------------------------------------
// Valida si la secuencia ingresada es la correcta
// -----------------------------------------------------------------------------
void validarSecuencia() {
    if (estado == 3) {
        ESP_LOGI(TAG, "APROBADO");  // Secuencia correcta
    } else {
        ESP_LOGI(TAG, "NO APROBADO");  // Secuencia incorrecta
    }
}

// -----------------------------------------------------------------------------
// Registra un toque (tipo: 3 = largo, 1 = corto)
// Verifica si es parte correcta de la secuencia
// -----------------------------------------------------------------------------
void registrarToque(int tipoToque) {
    if (estado < 3) {
        // Si el toque es del tipo correcto para la etapa actual
        if (tipoToque == secuenciaEsperada[estado]) {
            conteoToques++;  // Se registra el toque

            // Avanza al siguiente estado si ya se hicieron 3 toques válidos
            if ((estado == 0 || estado == 2) && conteoToques == 3 && tipoToque == 3) {
                estado++;
                conteoToques = 0;
            } else if (estado == 1 && conteoToques == 3 && tipoToque == 1) {
                estado++;
                conteoToques = 0;
            }
        } else {
            // Si el toque no corresponde con lo esperado, se reinicia
            estado = 0;
            conteoToques = 0;
            ESP_LOGI(TAG, "Secuencia incorrecta, reiniciando...");
        }
    }
}

// -----------------------------------------------------------------------------
// Función principal (app_main), punto de entrada en ESP-IDF
// -----------------------------------------------------------------------------
void app_main(void) {
    // Inicializa los pines táctiles
    touch_pad_init();
    touch_pad_config(TOUCH_PIN, 0);       // Configura pin de entrada principal
    touch_pad_config(VALIDAR_PIN, 0);     // Configura pin para validación

    ESP_LOGI(TAG, "Sistema iniciado. Esperando secuencia...");

    // Bucle principal del sistema
    while (1) {
        uint16_t lecturaToque = 0;
        uint16_t lecturaValidar = 0;

        // Lee el valor de los sensores táctiles
        touch_pad_read(TOUCH_PIN, &lecturaToque);
        touch_pad_read(VALIDAR_PIN, &lecturaValidar);

        // Si se detecta un toque en el pin principal (valor menor al umbral)
        if (lecturaToque < UMBRAL_TOQUE) {
            if (!tocando) {
                tocando = true;
                tiempoInicio = esp_timer_get_time() / 1000; // Guarda el momento en que inició el toque (ms)
            }
        } else {
            // Cuando se deja de tocar
            if (tocando) {
                tocando = false;
                int64_t duracion = (esp_timer_get_time() / 1000) - tiempoInicio;

                // Determina si fue un toque largo, corto o inválido
                if (duracion >= TOQUE_LARGO_MS) {
                    ESP_LOGI(TAG, "Toque largo detectado");
                    registrarToque(3);  // Registra como toque largo
                } else if (duracion >= TOQUE_CORTO_MS) {
                    ESP_LOGI(TAG, "Toque corto detectado");
                    registrarToque(1);  // Registra como toque corto
                } else {
                    ESP_LOGI(TAG, "Toque inválido (muy corto)");
                }
            }
        }

        // Si se toca el pin de validación
        if (lecturaValidar < UMBRAL_TOQUE) {
            vTaskDelay(300 / portTICK_PERIOD_MS); // Delay antirebote
            validarSecuencia();                   // Comprueba si la secuencia ingresada fue correcta
            resetearSecuencia();                  // Reinicia para ingresar una nueva secuencia
        }

        // Espera corta para evitar uso excesivo del procesador
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}
