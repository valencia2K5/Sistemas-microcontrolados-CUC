Código del Receptor ESP32:

#include <esp_now.h>         // Librería para usar el protocolo ESP-NOW
#include <WiFi.h>            // Librería para funciones de red Wi-Fi

#define LED 5                // Definimos el pin GPIO5 para el LED

// Configuración del canal PWM
const int frecuencia = 5000;     // Frecuencia de la señal PWM en Hz
const int canal = 0;             // Número del canal PWM (0-15 disponibles en ESP32)
const int resolucion = 12;       // Resolución del PWM en bits (12 bits = 0 a 4095)

// Definición de la estructura para recibir los datos del emisor
typedef struct struct_message {
  char a[32];            // Cadena de texto (hasta 31 caracteres + '\0')
  int raw;               // Valor entero (ej. valor crudo del ADC)
  float Porcentaje;      // Valor en porcentaje
  bool d;                // Valor booleano
} struct_message;

// Creamos una variable de tipo struct_message para almacenar los datos recibidos
struct_message Datos;

// Función callback que se ejecuta automáticamente al recibir datos vía ESP-NOW
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  // Copia los datos recibidos del buffer 'incomingData' a la estructura 'Datos'
  memcpy(&Datos, incomingData, sizeof(Datos));

  // Imprime en el monitor serial los datos recibidos
  Serial.print("Data received: ");
  Serial.println(len);
  Serial.print("Character Value: ");
  Serial.println(Datos.a);
  Serial.print("Integer Value: ");
  Serial.println(Datos.raw);
  Serial.print("Float Value: ");
  Serial.println(Datos.Porcentaje);
  Serial.print("Boolean Value: ");
  Serial.println(Datos.d);
  Serial.println();

  // Convierte el valor recibido (raw) en un duty cycle válido para el PWM
  int duty = constrain(Datos.raw, 0, 4095);  // Asegura que el valor esté entre 0 y 4095

  // Aplica el valor PWM al LED usando ledcWrite
  ledcWrite(canal, duty);  // Cambia el brillo del LED según el duty cycle recibido
}

/*---------------------------------------------------------------------------------------------------------------------------*/

void setup() {
  Serial.begin(115200);           // Inicializa la comunicación serial a 115200 baudios
  WiFi.mode(WIFI_STA);            // Configura el ESP32 en modo estación (no actúa como AP)

  // Inicializa el protocolo ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error inicializando ESP-NOW");  // Mensaje de error si no se inicializa
    return;  // Sale del setup si hubo error
  }

  // Registra la función callback para manejar los datos recibidos
  esp_now_register_recv_cb(OnDataRecv);

  // Configura el canal PWM con frecuencia y resolución especificadas
  ledcSetup(canal, frecuencia, resolucion);

  // Asocia el canal PWM al pin GPIO definido (LED)
  ledcAttachPin(LED, canal);
}

void loop() {
  // No se necesita lógica en el loop, ya que el control del LED ocurre en la función callback
}
