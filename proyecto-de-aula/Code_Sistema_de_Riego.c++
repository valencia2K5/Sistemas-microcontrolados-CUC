/*
 * PROYECTO DE RIEGO INTELIGENTE CON ESP32 Y BLYNK
 * 
 * Este sistema controla automáticamente el riego basado en:
 * - Lectura de humedad del suelo
 * - Control manual mediante botón físico
 * - Control remoto via Blynk
 * - Protección contra riegos demasiado frecuentes
 */

// ==============================================
// SECCIÓN 1: INCLUSIÓN DE LIBRERÍAS
// ==============================================
#include <Arduino.h>       // Librería básica de Arduino (necesaria en PlatformIO)
#include <WiFi.h>          // Para conexión WiFi del ESP32
#include <BlynkSimpleEsp32.h> // Para integración con Blynk IoT

// ==============================================
// SECCIÓN 2: CONFIGURACIÓN - CONSTANTES Y CREDENCIALES
// ==============================================

// Credenciales Blynk (obtener desde la app Blynk IoT)
#define BLYNK_TEMPLATE_ID "TMPLXXXXXX"       // ID de plantilla Blynk
#define BLYNK_TEMPLATE_NAME "Riego Inteligente" // Nombre visible en app
#define BLYNK_AUTH_TOKEN "TuTokenDeBlynk"    // Token de autenticación único

// Credenciales de tu red WiFi
char ssid[] = "TuRedWiFi";           // Nombre (SSID) de tu red WiFi
char pass[] = "TuPasswordWiFi";      // Contraseña de tu WiFi

// Configuración de hardware - Asignación de pines
const int PIN_SENSOR_HUMEDAD = 34;  // GPIO34 (ADC1_CH6) para sensor de humedad
const int PIN_RELE = 5;             // GPIO5 para controlar el relé
const int PIN_BOTON_MANUAL = 4;     // GPIO4 para botón manual (con pull-up interno)

// ==============================================
// SECCIÓN 3: VARIABLES GLOBALES
// ==============================================
int humedadLimite = 30;       // % de humedad para activar riego (ajustable via Blynk)
int humedadActual = 0;        // Almacena la lectura actual del sensor
bool modoAutomatico = true;   // true = modo automático, false = manual
bool estadoRiego = false;     // true = riego activado, false = desactivado
unsigned long ultimoRiego = 0; // Marca de tiempo del último riego (en ms)
const long INTERVALO_MIN_RIEGO = 1800000; // Tiempo mínimo entre riegos (30 minutos)

// ==============================================
// SECCIÓN 4: PROTOTIPOS DE FUNCIONES
// (Declaraciones para que el compilador conozca las funciones)
// ==============================================
void leerHumedad();           // Lee el sensor de humedad
void activarRiego();          // Activa el sistema de riego
void desactivarRiego();       // Desactiva el sistema de riego
void controlAutomatico();     // Lógica de control automático
void verificarBotonManual();  // Revisa el estado del botón físico
void enviarDatosBlynk();      // Envía datos a la app Blynk

// ==============================================
// SECCIÓN 5: FUNCIONES PRINCIPALES
// ==============================================

/**
 * Lee el sensor de humedad y convierte el valor a porcentaje
 */
void leerHumedad() {
  // Leer valor analógico del sensor (0-4095 en ESP32)
  int lectura = analogRead(PIN_SENSOR_HUMEDAD);
  
  /**
   * Convertir lectura a porcentaje:
   * - map() reescala el valor de entrada (1500-3500) a (100-0)
   * - constrain() asegura que el resultado esté entre 0-100%
   * NOTA: Los valores 1500 y 3500 deben calibrarse según tu sensor específico
   */
  humedadActual = map(lectura, 1500, 3500, 100, 0); 
  humedadActual = constrain(humedadActual, 0, 100);
  
  // Mostrar valor por monitor serial para depuración
  Serial.print("Humedad: ");
  Serial.print(humedadActual);
  Serial.println("%");
}

/**
 * Activa el sistema de riego
 */
void activarRiego() {
  // Verificar que el riego no esté ya activado
  if (!estadoRiego) {
    digitalWrite(PIN_RELE, LOW);  // Activar relé (LOW para módulos activos por bajo)
    estadoRiego = true;
    ultimoRiego = millis();       // Registrar momento de activación
    
    // Debug y notificaciones
    Serial.println("Riego ACTIVADO");
    Blynk.virtualWrite(V1, 1);   // Actualizar LED virtual en Blynk (1 = encendido)
    Blynk.logEvent("riego_activado", String("Riego activado. Humedad: ") + humedadActual + "%");
  }
}

/**
 * Desactiva el sistema de riego
 */
void desactivarRiego() {
  // Verificar que el riego esté activado
  if (estadoRiego) {
    digitalWrite(PIN_RELE, HIGH);  // Desactivar relé
    estadoRiego = false;
    
    // Debug y notificaciones
    Serial.println("Riego DESACTIVADO");
    Blynk.virtualWrite(V1, 0);    // Actualizar LED virtual en Blynk (0 = apagado)
  }
}

/**
 * Control automático basado en humedad y tiempo
 */
void controlAutomatico() {
  if (modoAutomatico) {
    // Condiciones para activar riego:
    // 1. Riego no está activo
    // 2. Humedad está por debajo del límite
    // 3. Ha pasado el tiempo mínimo desde el último riego
    if (!estadoRiego && humedadActual < humedadLimite && 
        (millis() - ultimoRiego) > INTERVALO_MIN_RIEGO) {
      activarRiego();
    }
    // Condiciones para desactivar riego:
    // 1. Riego está activo
    // 2. Humedad está 5% por encima del límite (para evitar oscilaciones)
    else if (estadoRiego && humedadActual > (humedadLimite + 5)) {
      desactivarRiego();
    }
  }
}

/**
 * Verifica el estado del botón manual
 */
void verificarBotonManual() {
  // El botón está conectado con pull-up, por lo que LOW significa presionado
  if (digitalRead(PIN_BOTON_MANUAL) == LOW) {
    delay(50); // Pequeña pausa para eliminar "rebotes" (debounce)
    
    // Verificar nuevamente para confirmar que fue una pulsación real
    if (digitalRead(PIN_BOTON_MANUAL) == LOW) {
      // Alternar estado de riego
      if (estadoRiego) desactivarRiego();
      else activarRiego();
      
      // Esperar hasta que se suelte el botón
      while (digitalRead(PIN_BOTON_MANUAL) == LOW); 
    }
  }
}

/**
 * Envía datos a la app Blynk
 */
void enviarDatosBlynk() {
  Blynk.virtualWrite(V0, humedadActual);       // Enviar humedad al Gauge (V0)
  Blynk.virtualWrite(V3, modoAutomatico ? 1 : 0); // Enviar estado de modo (V3)
  Blynk.virtualWrite(V4, humedadLimite);       // Enviar límite de humedad (V4)
}

// ==============================================
// SECCIÓN 6: HANDLERS DE BLYNK
// (Funciones que responden a interacciones en la app)
// ==============================================

/**
 * Handler para el botón virtual de riego (V2)
 */
BLYNK_WRITE(V2) {
  // param.asInt() devuelve 1 si está activado, 0 si no
  if (param.asInt()) activarRiego();
  else desactivarRiego();
}

/**
 * Handler para el switch de modo auto/manual (V3)
 */
BLYNK_WRITE(V3) {
  modoAutomatico = param.asInt(); // 1 = automático, 0 = manual
}

/**
 * Handler para el slider de humedad límite (V4)
 */
BLYNK_WRITE(V4) {
  humedadLimite = param.asInt(); // Actualizar valor del límite
}

/**
 * Evento cuando se conecta a Blynk
 */
BLYNK_CONNECTED() {
  // Sincronizar valores de los widgets V3 (modo) y V4 (humedad límite)
  Blynk.syncVirtual(V3, V4);
}

// ==============================================
// SECCIÓN 7: SETUP Y LOOP PRINCIPAL
// ==============================================

/**
 * Configuración inicial (se ejecuta una vez al iniciar)
 */
void setup() {
  // Iniciar comunicación serial para depuración (115200 baudios)
  Serial.begin(115200);
  
  // Configurar pines de hardware
  pinMode(PIN_RELE, OUTPUT);        // Configurar pin del relé como salida
  pinMode(PIN_BOTON_MANUAL, INPUT_PULLUP); // Configurar botón con pull-up interno
  digitalWrite(PIN_RELE, HIGH);     // Asegurar que el relé inicia apagado
  
  // Conectar a Blynk (se bloquea hasta que se conecte o timeout)
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Conectado a Blynk!");
}

/**
 * Bucle principal (se ejecuta continuamente)
 */
void loop() {
  // Procesar eventos de Blynk (requerido para mantener conexión)
  Blynk.run();
  
  // Variable estática para mantener el valor entre llamadas a loop()
  static unsigned long ultimaLectura = 0;
  
  // Ejecutar lecturas y controles cada 2000ms (2 segundos)
  if (millis() - ultimaLectura > 2000) {
    leerHumedad();        // Leer sensor
    controlAutomatico();  // Ejecutar lógica automática
    enviarDatosBlynk();   // Actualizar app móvil
    ultimaLectura = millis(); // Actualizar marca de tiempo
  }
  
  // Verificar estado del botón manual en cada iteración
  verificarBotonManual();
  
  // Pequeña pausa para evitar sobrecarga del procesador
  delay(10);
}