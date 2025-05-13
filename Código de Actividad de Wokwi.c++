/*
  Controlador de temperatura con ESP32
  Características:
  - Lectura de temperatura con sensor DHT22
  - Setpoint ajustable por potenciómetro o monitor serial
  - Visualización en pantalla OLED y LCD
  - Control ON-OFF con histéresis
*/

// ==================== BIBLIOTECAS ====================
#include <Wire.h>              // Para comunicación I2C
#include <Adafruit_SSD1306.h>  // Controlador pantalla OLED
#include <LiquidCrystal_I2C.h> // Controlador LCD I2C
#include <DHT.h>              // Sensor de temperatura/humedad

// ==================== CONFIGURACIÓN DE HARDWARE ====================
#define DHTPIN 4        // Pin GPIO4 para el sensor DHT
#define DHTTYPE DHT22   // Tipo de sensor DHT22
#define OUTPUT_PIN 2    // Pin para el LED de salida
#define POT_PIN 34      // Pin para el potenciómetro (ADC1_CH6)

// Configuración pantalla OLED (128x64)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C  // Dirección I2C

// Configuración pantalla LCD (16x2)
#define LCD_ADDR 0x27   // Dirección I2C
#define LCD_COLS 16     // 16 caracteres por línea
#define LCD_ROWS 2      // 2 líneas

// ==================== DECLARACIÓN DE OBJETOS ====================
DHT dht(DHTPIN, DHTTYPE); // Objeto para el sensor DHT
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // Objeto OLED
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS); // Objeto LCD

// ==================== VARIABLES GLOBALES ====================
float temperature = 0;     // Almacena la temperatura actual
float setpoint = 0;        // Valor de setpoint actual
bool outputState = false;  // Estado de la salida (true = ON)
bool usePotentiometer = true; // true=usa potenciómetro, false=usa serial

// ==================== PROTOTIPOS DE FUNCIONES ====================
void processSerialCommands(); // Procesa comandos del monitor serial
void updateDisplays();        // Actualiza las pantallas OLED y LCD

// ==================== SETUP (INICIALIZACIÓN) ====================
void setup() {
  Serial.begin(115200); // Inicia comunicación serial a 115200 baudios
  
  // Inicialización del sensor DHT22
  dht.begin();
  
  // Configuración pantalla OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("Error al iniciar OLED"));
    while(1); // Bloquea el programa si hay error
  }
  display.display(); // Muestra buffer inicial
  delay(2000);      // Espera 2 segundos
  
  // Configuración pantalla LCD
  lcd.init();       // Inicializa la LCD
  lcd.backlight();  // Enciende la retroiluminación
  
  // Configuración del pin de salida
  pinMode(OUTPUT_PIN, OUTPUT);
  digitalWrite(OUTPUT_PIN, LOW);
  
  // Mensaje inicial en OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Sistema Control ON-OFF");
  display.setCursor(0,20);
  display.println("ESP32 + DHT22");
  display.display();
  
  // Mensaje inicial en LCD
  lcd.setCursor(0,0);
  lcd.print("Iniciando...");
  lcd.setCursor(0,1);
  lcd.print("ESP32 + DHT22");
  
  delay(2000); // Espera inicial de 2 segundos
}

// ==================== LOOP (PROGRAMA PRINCIPAL) ====================
void loop() {
  // 1. LECTURA DE TEMPERATURA
  temperature = dht.readTemperature(); // Lee temperatura en °C
  
  // 2. LECTURA DE SETPOINT (POTENCIÓMETRO O SERIAL)
  if(usePotentiometer) {
    // Mapea el valor analógico (0-4095) a rango 0-100°C
    setpoint = map(analogRead(POT_PIN), 0, 4095, 0, 100);
  }
  
  // 3. CONTROL ON-OFF CON HISTÉRESIS (±0.5°C)
  if(temperature > setpoint + 0.5) { // Si temperatura > setpoint + 0.5°C
    digitalWrite(OUTPUT_PIN, HIGH);  // Activa salida
    outputState = true;
  } 
  else if(temperature < setpoint - 0.5) { // Si temperatura < setpoint - 0.5°C
    digitalWrite(OUTPUT_PIN, LOW);   // Desactiva salida
    outputState = false;
  }
  
  // 4. PROCESAMIENTO DE COMANDOS SERIAL
  processSerialCommands();
  
  // 5. ACTUALIZACIÓN DE PANTALLAS
  updateDisplays();
  
  delay(1000); // Espera 1 segundo entre lecturas
}

// ==================== FUNCIÓN PARA COMANDOS SERIAL ====================
void processSerialCommands() {
  if(Serial.available() > 0) { // Si hay datos disponibles
    String input = Serial.readStringUntil('\n'); // Lee hasta salto de línea
    input.trim(); // Elimina espacios en blanco
    
    // Comando para establecer setpoint: "set=XX.X"
    if(input.startsWith("set=")) {
      float newSetpoint = input.substring(4).toFloat(); // Extrae el valor
      if(newSetpoint >= 0 && newSetpoint <= 100) { // Verifica rango válido
        setpoint = newSetpoint;
        usePotentiometer = false; // Cambia a modo serial
        Serial.print("Setpoint actualizado: ");
        Serial.print(setpoint);
        Serial.println("°C");
      } else {
        Serial.println("Error: Rango válido 0-100°C");
      }
    }
    // Comando para usar potenciómetro: "pot"
    else if(input == "pot") {
      usePotentiometer = true;
      Serial.println("Modo potenciómetro activado");
    }
    // Comando de ayuda: "help"
    else if(input == "help") {
      Serial.println("Comandos disponibles:");
      Serial.println("set=XX.X - Establece setpoint (0-100°C)");
      Serial.println("pot - Usar potenciómetro");
      Serial.println("help - Muestra esta ayuda");
    }
  }
}

// ==================== FUNCIÓN PARA ACTUALIZAR PANTALLAS ====================
void updateDisplays() {
  // --- PANTALLA OLED ---
  display.clearDisplay(); // Limpia el buffer
  
  // Muestra temperatura actual
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Temp: ");
  display.print(temperature, 1); // 1 decimal
  display.print("°C");
  
  // Muestra setpoint y modo
  display.setCursor(0,20);
  display.print("Set: ");
  display.print(setpoint, 1);
  display.print("°C (");
  display.print(usePotentiometer ? "POT" : "SER");
  display.println(")");
  
  // Muestra estado de salida
  display.setCursor(0,40);
  display.print("Salida: ");
  display.print(outputState ? "ON" : "OFF");
  
  display.display(); // Actualiza pantalla
  
  // --- PANTALLA LCD ---
  lcd.clear();
  
  // Primera línea: Temperatura y Setpoint
  lcd.setCursor(0,0);
  lcd.print("T:");
  lcd.print(temperature, 1);
  lcd.print(" S:");
  lcd.print(setpoint, 1);
  
  // Segunda línea: Modo y Estado
  lcd.setCursor(0,1);
  lcd.print("Modo:");
  lcd.print(usePotentiometer ? "POT" : "SER");
  lcd.print(" ");
  lcd.print(outputState ? "ON" : "OFF");
}