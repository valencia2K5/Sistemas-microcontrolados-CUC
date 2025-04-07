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
#include "driver/touch_pad.h"
#include "esp_log.h"
#include "esp_timer.h"

#define TOUCH_PIN      TOUCH_PAD_NUM0  // GPIO 4
#define VALIDAR_PIN    TOUCH_PAD_NUM3  // GPIO 15

#define TOQUE_CORTO_MS 1000
#define TOQUE_LARGO_MS 3000
#define UMBRAL_TOQUE   30

static const char *TAG = "TOQUE";

int secuenciaEsperada[] = {3, 1, 3};
int estado = 0;
int conteoToques = 0;

bool tocando = false;
int64_t tiempoInicio = 0;

void resetearSecuencia() {
    estado = 0;
    conteoToques = 0;
}

void validarSecuencia() {
    if (estado == 3) {
        ESP_LOGI(TAG, "APROBADO");
    } else {
        ESP_LOGI(TAG, "NO APROBADO");
    }
}

void registrarToque(int tipoToque) {
    if (estado < 3) {
        if (tipoToque == secuenciaEsperada[estado]) {
            conteoToques++;
            if ((estado == 0 || estado == 2) && conteoToques == 3 && tipoToque == 3) {
                estado++;
                conteoToques = 0;
            } else if (estado == 1 && conteoToques == 3 && tipoToque == 1) {
                estado++;
                conteoToques = 0;
            }
        } else {
            estado = 0;
            conteoToques = 0;
            ESP_LOGI(TAG, "Secuencia incorrecta, reiniciando...");
        }
    }
}

void app_main(void) {
    touch_pad_init();
    touch_pad_config(TOUCH_PIN, 0);
    touch_pad_config(VALIDAR_PIN, 0);

    ESP_LOGI(TAG, "Sistema iniciado. Esperando secuencia...");

    while (1) {
        uint16_t lecturaToque = 0;
        uint16_t lecturaValidar = 0;

        touch_pad_read(TOUCH_PIN, &lecturaToque);
        touch_pad_read(VALIDAR_PIN, &lecturaValidar);

        if (lecturaToque < UMBRAL_TOQUE) {
            if (!tocando) {
                tocando = true;
                tiempoInicio = esp_timer_get_time() / 1000;
            }
        } else {
            if (tocando) {
                tocando = false;
                int64_t duracion = (esp_timer_get_time() / 1000) - tiempoInicio;

                if (duracion >= TOQUE_LARGO_MS) {
                    ESP_LOGI(TAG, "Toque largo detectado");
                    registrarToque(3);
                } else if (duracion >= TOQUE_CORTO_MS) {
                    ESP_LOGI(TAG, "Toque corto detectado");
                    registrarToque(1);
                } else {
                    ESP_LOGI(TAG, "Toque inválido (muy corto)");
                }
            }
        }

        if (lecturaValidar < UMBRAL_TOQUE) {
            vTaskDelay(300 / portTICK_PERIOD_MS); // antirebote
            validarSecuencia();
            resetearSecuencia();
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}
