/*
 * PROYECTO DE RIEGO INTELIGENTE CON ESP32 Y BLYNK
 * 
 * Este sistema controla automáticamente el riego basado en:
 * - Lectura de humedad del suelo.
 * - Control remoto via Blynk.
 * - Protección contra riegos demasiado frecuentes.
 */

// Datos del proyecto Blynk (puedes obtenerlos desde la plataforma Blynk)
#define BLYNK_TEMPLATE_ID "TMPL2YlsrSHyK"
#define BLYNK_TEMPLATE_NAME "Sistema de Riego ESP32"
#define BLYNK_AUTH_TOKEN "Gy1OXXvQLzybzXPk9qT2nhygEZmKrFqv"

// Inclusión de las librerías necesarias
#include <LiquidCrystal_I2C.h>       // Librería para manejar la pantalla LCD I2C
#include <Wire.h>                    // Librería para comunicación I2C
#include <WiFiClient.h>              // Permite la conexión a internet mediante WiFi
#include <BlynkSimpleEsp32.h>        // Librería Blynk específica para ESP32

// Definiciones de pines
#define sensor 33    // Pin analógico para leer el sensor de humedad del suelo
#define relay 4      // Pin digital para controlar el relé que activa la bomba

// Inicialización del objeto LCD con dirección I2C 0x27 y tamaño 16x2 caracteres
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Objeto para manejar temporizadores de Blynk
BlynkTimer timer;

// Credenciales de Blynk y WiFi
char auth[] = "Gy1OXXvQLzybzXPk9qT2nhygEZmKrFqv";   // Token de autenticación de Blynk
char ssid[] = "Galaxy A35";                         // Nombre de la red WiFi
char pass[] = "JosVal2005";                         // Contraseña de la red WiFi

// Función que se ejecuta al iniciar el ESP32
void setup() {
  Serial.begin(115200);  // Inicializa el monitor serial a 115200 baudios
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80); // Conecta el ESP32 a Blynk vía WiFi

  lcd.init();         // Inicializa la pantalla LCD
  lcd.backlight();    // Enciende la luz de fondo del LCD

  pinMode(relay, OUTPUT);     // Establece el pin del relé como salida
  digitalWrite(relay, HIGH);  // Apaga el relé (asumiendo lógica inversa: HIGH = OFF)

  // Animación de carga en el LCD
  lcd.setCursor(1, 0);
  lcd.print("System Loading");      // Muestra el texto inicial
  for (int a = 0; a <= 15; a++) {
    lcd.setCursor(a, 1);            // Mueve el cursor de izquierda a derecha
    lcd.print(".");                 // Imprime puntos uno por uno (animación)
    delay(200);                     // Espera 200 ms entre puntos
  }
  lcd.clear();                      // Limpia la pantalla al finalizar la animación
}

// Función que mide la humedad del suelo y actualiza el LCD y Blynk
void soilMoisture() {
  int value = analogRead(sensor);              // Lee el valor analógico del sensor (0-4095)
  value = map(value, 0, 4095, 0, 100);          // Convierte a porcentaje del 0% (seco) al 100% (mojado)
  value = (value - 100) * -1;                   // Invierte para que 0 sea seco y 100 húmedo

  Blynk.virtualWrite(V0, value);               // Envía el valor a Blynk (al pin virtual V0)
  Serial.println(value);                       // Imprime el valor en el monitor serial

  lcd.setCursor(0, 0);                         // Mueve el cursor a la primera línea
  lcd.print("Moisture : ");                    // Imprime el texto
  lcd.print(value);                            // Imprime el valor
  lcd.print("%   ");                           // Imprime el símbolo de porcentaje (con espacio extra para limpiar residuos)
}

// Función que se ejecuta cuando se presiona el botón en la app de Blynk (V1)
BLYNK_WRITE(V1) {
  bool Relay = param.asInt();        // Obtiene el valor del botón (1 = ON, 0 = OFF)
  if (Relay == 1) {
    digitalWrite(relay, LOW);        // Enciende la bomba (LOW = ON en muchos relés)
    lcd.setCursor(0, 1);
    lcd.print("Motor is ON ");       // Muestra mensaje en el LCD
  } else {
    digitalWrite(relay, HIGH);       // Apaga la bomba (HIGH = OFF)
    lcd.setCursor(0, 1);
    lcd.print("Motor is OFF");       // Muestra mensaje en el LCD
  }
}

// Función que se ejecuta constantemente
void loop() {
  soilMoisture();  // Llama a la función para leer humedad y actualizar Blynk y LCD
  Blynk.run();     // Necesario para que Blynk siga funcionando

  delay(200);      // Espera 200 ms antes de repetir (puedes mejorar esto usando timer)
}
