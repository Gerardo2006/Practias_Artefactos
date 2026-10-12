#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// ---------------------- CONFIGURACIÓN WI-FI ----------------------
#define WLAN_SSID "Gerardo"
#define WLAN_PASS "12345678"

// ---------------------- CONFIGURACIÓN ADAFRUIT IO ----------------------
#define AIO_SERVER     "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME  "Gerardo2006"
#define AIO_KEY       ""

// ---------------------- CLIENTE MQTT Y FEED ----------------------
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish feedDistancia = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/distancia");

// ---------------------- AJUSTES ----------------------
#define INTERVALO_PUBLICAR_MS 5000
#define TRIG_PIN 33
#define ECHO_PIN 32

// Variables globales
unsigned long ultimoPublicar = 0;
float ultimaDistancia = 0;

// ---------------------- PROTOTIPOS ----------------------
void conectarWiFi();
void conectarMQTT();
float medirDistancia();
float distanciaPromedio(int muestras);

// =====================================================================
void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(TRIG_PIN, LOW);

  conectarWiFi();
}

// =====================================================================
void loop() {
  conectarMQTT();  // reconecta si se cae

  // Publicar cada INTERVALO_PUBLICAR_MS
  if (millis() - ultimoPublicar >= INTERVALO_PUBLICAR_MS) {
    ultimoPublicar = millis();

    float d = distanciaPromedio(5);

    if (d > 0) {
      ultimaDistancia = d;
      Serial.print("Distancia: ");
      Serial.print(d, 1);
      Serial.println(" cm");

      if (!feedDistancia.publish(d)) {
        Serial.println("Error al publicar la distancia");
      }
    } else {
      Serial.println("Lectura fuera de rango o sin eco");
    }
  }

  // Pequeña pausa para no saturar el loop
  delay(50);
}

// =====================================================================
//  MEDICIÓN ULTRASÓNICA
// =====================================================================
float medirDistancia() {
  // Pulso de 10 µs en TRIG
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Leer duración del eco (timeout 30 ms ≈ 5 m máx)
  long duracion = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duracion == 0) return -1;  // sin eco

  // Velocidad del sonido: 0.0343 cm/µs
  return duracion * 0.0343 / 2.0;
}

float distanciaPromedio(int muestras) {
  float suma = 0;
  int validas = 0;
  for (int i = 0; i < muestras; i++) {
    float d = medirDistancia();
    if (d > 0) {
      suma += d;
      validas++;
    }
    delay(60);  // esperar entre mediciones (evita ecos cruzados)
  }
  if (validas == 0) return -1;
  return suma / validas;
}

// =====================================================================
//  CONEXIONES
// =====================================================================
void conectarWiFi() {
  Serial.print("Conectando a ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi conectado. IP: ");
  Serial.println(WiFi.localIP());
}

void conectarMQTT() {
  if (mqtt.connected()) return;

  Serial.print("Conectando a Adafruit IO... ");
  int8_t ret;
  uint8_t intentos = 3;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Reintentando en 5 segundos...");
    mqtt.disconnect();
    delay(5000);
    if (--intentos == 0) {
      Serial.println("No se pudo conectar. Reiniciando la ESP32...");
      ESP.restart();
    }
  }
  Serial.println("¡Conectado a Adafruit IO!");
}
