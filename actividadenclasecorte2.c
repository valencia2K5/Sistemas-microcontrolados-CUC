#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"

#define TOUCH_PIN1 GPIO_NUM_13     // Cambiar el pin táctil a GPIO_NUM_13
#define TOUCH_PIN2 GPIO_NUM_4      // Mantener el pin táctil 2 en GPIO_NUM_4
#define ADC_CHANNEL ADC1_CHANNEL_6 // GPIO34 por ejemplo

void app_main(void)
{
    // Configurar ADC (canal 6 corresponde a GPIO34 en ESP32)
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_11);

    // Configurar pines táctiles como entrada
    gpio_set_direction(TOUCH_PIN1, GPIO_MODE_INPUT);
    gpio_set_direction(TOUCH_PIN2, GPIO_MODE_INPUT);

    // Mensaje inicial por serial
    printf("\n--- Sistema iniciado ---\n");
    printf("Escribe '1' y presiona Enter para comenzar:\n");

    // Esperar que llegue '1' por serial
    char c = 0;
    while (c != '1') {
        int r = scanf("%c", &c);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    while (1) {
        // Verificar si se ha presionado algún pin táctil
        if (gpio_get_level(TOUCH_PIN1) == 0 || gpio_get_level(TOUCH_PIN2) == 0) {
            int adc_val = adc1_get_raw(ADC_CHANNEL);
            float porcentaje = (adc_val / 4095.0) * 100.0;

            printf("ADC: %d - Porcentaje: %.2f%%\n", adc_val, porcentaje);
            vTaskDelay(1000 / portTICK_PERIOD_MS); // Esperar 1 segundo
        }
    }
}
