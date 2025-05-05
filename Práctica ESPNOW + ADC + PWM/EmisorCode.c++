Código del Emisor ESP32 (Potenciómetro):

#include <Arduino.h>             // Librería principal de Arduino para funciones básicas
#include <WiFi.h>                // Librería para funciones de Wi-Fi
#include <esp_now.h>             // Librería para protocolo ESP-NOW
#include <esp_wifi.h>            // Librería adicional para configuración Wi-Fi de bajo nivel
#include "driver/adc.h"          // Librería para configurar el ADC de bajo nivel

// Definimos el pin del potenciómetro como ADC1_CHANNEL_6 (GPIO34)
#define Potenciometro ADC1_CHANNEL_6
#define ADC_WIDTH ADC_WIDTH_BIT_12  // Resolución del ADC a 12 bits (valores de 0 a 4095)
#define ADC_ATTEN ADC_ATTEN_DB_11   // Atenuación de 11 dB (permite leer hasta ~3.3V)

const float ADC_VOLT_MAX = 3.3;  // Voltaje máximo de referencia del ADC (no se usa directamente aquí)

/*ESP Transmisor 
MAC:e4:65:b8:47:fc:e8*/

/*ESP RECEPTOR 
MAC:fc:b4:67:78:c2:d4*/

// Dirección MAC del receptor (peer) a la cual se enviarán los datos
uint8_t broadcastAddress[] = {0x5C, 0x01, 0x3B, 0x72, 0x58, 0x7C};

// Definimos la estructura de los datos que se enviarán mediante ESP-NOW
typedef struct estructura_del_mensaje {
    char a[32];  // Cadena de texto de hasta 31 caracteres + terminador nulo
    int b;       // Valor entero (por ejemplo, lectura cruda del potenciómetro)
    float c;     // Valor flotante (porcentaje u otro cálculo)
    bool d;      // Valor booleano (bandera de control, si se quiere)
} estructura_del_mensaje;

estructura_del_mensaje Datos;  // Instancia de la estructura para almacenar los datos a enviar

esp_now_peer_info_t peerInfo;  // Estructura para guardar información del peer (receptor)

// Callback que se ejecuta cuando se completa un envío
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("\r\nEstatus del envio del mensaje: \t");
    // Imprime si el envío fue exitoso o fallido
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Envio completado" : "Envio fallido");
}

void setup() {
    Serial.begin(115200);  // Inicializa la comunicación serial a 115200 baudios
    
    // Configura el ADC de 12 bits para leer desde GPIO34 (canal 6 de ADC1)
    adc1_config_width(ADC_WIDTH);
    adc1_config_channel_atten(Potenciometro, ADC_ATTEN);
    
    // Configura el ESP32 como estación WiFi (no se conecta a ninguna red)
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();  // Se asegura de que no haya conexiones WiFi activas
    
    // Inicializa ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error al iniciar ESP NOW");
        return;  // Detiene el setup si hubo error
    }
    
    // Registra la función de callback que se ejecutará después de cada intento de envío
    esp_now_register_send_cb(OnDataSent);
    
    // Configura la información del peer (receptor) al que se enviarán los datos
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);  // Copia la dirección MAC del receptor
    peerInfo.channel = 0;                             // Canal Wi-Fi (0 usa el canal actual)
    peerInfo.encrypt = false;                         // Comunicación sin cifrado
    
    // Añade el peer (receptor) a la lista de dispositivos conocidos
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Error al agregar semilla");  // Mensaje en caso de error
        return;
    }
    
    Serial.println("Emisor listo en GPIO34");  // Confirmación en el monitor serial
}

void loop() {
    int raw = adc1_get_raw(Potenciometro);  // Lee el valor crudo del potenciómetro (0-4095)
    float porcentaje = (raw / 4095.0) * 100.0;  // Convierte ese valor a porcentaje (0-100%)
    
    // Llena la estructura con los datos a enviar
    strcpy(Datos.a, "Valor ADC GPIO34");  // Texto descriptivo
    Datos.b = raw;                        // Valor crudo leído del ADC
    Datos.c = porcentaje;                 // Porcentaje calculado
    Datos.d = true;                       // Bandera en true (puede usarse como indicador de estado)

    // Envía los datos mediante ESP-NOW al receptor
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &Datos, sizeof(Datos));
    
    // Imprime si el envío fue exitoso
    if (result == ESP_OK) {
        Serial.print("Enviado - Valor: ");
        Serial.print(raw);
        Serial.print(" (");
        Serial.print(porcentaje);
        Serial.println("%)");
    } else {
        Serial.println("Error al enviar");  // Mensaje de error si el envío falla
    }
    
    delay(500);  // Espera medio segundo antes del siguiente envío
}
