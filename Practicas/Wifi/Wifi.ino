#include <WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

// Credenciales de tu red Wi-Fi
#define WLAN_SSID "CLARO_2.4GHz_0BDAEC"
#define WLAN_PASS "E9ADB07197"

// Credenciales de Adafruit IO
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME "Gerardo2006"
#define AIO_KEY ""

// PINES
#define PIN_B 32
#define PIN_O 33
#define PIN_Y 25
#define PIN_G 26
#define PIN_R 27
#define PIN_LM35 34

// Variables globales para Wi-Fi y MQTT
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// --- CONFIGURACIÓN DE FEEDS ---
Adafruit_MQTT_Publish pub_feed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperatura");
Adafruit_MQTT_Subscribe sub_feed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/boton");

// Variables globales de estado y temperatura
bool boton = false;
float temperaturaCelsius = 0.0;

// ============================================
// FUNCIÓN GLOBAL (fuera de setup, aquí es su lugar)
// ============================================
float leerTemperaturaCelsius() {
  uint32_t sumaMv = 0;
  const int NUM_MUESTRAS = 30;

  for (int i = 0; i < NUM_MUESTRAS; i++) {
    sumaMv += analogReadMilliVolts(PIN_LM35);  // lectura calibrada en mV
    delay(2);
  }

  float mV = sumaMv / (float)NUM_MUESTRAS;
  return mV / 10.0;  // LM35: 10 mV por cada °C
}

// Función para actualizar los LEDs externos
void actualizarLED() {
  if (boton == false) {
    digitalWrite(PIN_B, LOW);
    digitalWrite(PIN_O, LOW);
    digitalWrite(PIN_Y, LOW);
    digitalWrite(PIN_G, LOW);
    digitalWrite(PIN_R, LOW);
    return;
  }

  if (temperaturaCelsius > 40) {
    digitalWrite(PIN_B, HIGH);
    digitalWrite(PIN_O, HIGH);
    digitalWrite(PIN_Y, HIGH);
    digitalWrite(PIN_G, HIGH);
    digitalWrite(PIN_R, HIGH);
  } else if (temperaturaCelsius > 32) {
    digitalWrite(PIN_B, LOW);
    digitalWrite(PIN_O, HIGH);
    digitalWrite(PIN_Y, HIGH);
    digitalWrite(PIN_G, HIGH);
    digitalWrite(PIN_R, HIGH);
  } else if (temperaturaCelsius > 24) {
    digitalWrite(PIN_B, LOW);
    digitalWrite(PIN_O, LOW);
    digitalWrite(PIN_Y, HIGH);
    digitalWrite(PIN_G, HIGH);
    digitalWrite(PIN_R, HIGH);
  } else if (temperaturaCelsius > 16) {
    digitalWrite(PIN_B, LOW);
    digitalWrite(PIN_O, LOW);
    digitalWrite(PIN_Y, LOW);
    digitalWrite(PIN_G, HIGH);
    digitalWrite(PIN_R, HIGH);
  } else if (temperaturaCelsius > 8) {
    digitalWrite(PIN_B, LOW);
    digitalWrite(PIN_O, LOW);
    digitalWrite(PIN_Y, LOW);
    digitalWrite(PIN_G, LOW);
    digitalWrite(PIN_R, HIGH);
  } else {
    digitalWrite(PIN_B, LOW);
    digitalWrite(PIN_O, LOW);
    digitalWrite(PIN_Y, LOW);
    digitalWrite(PIN_G, LOW);
    digitalWrite(PIN_R, LOW);
  }
}

// Función callback para la suscripción
void interruptorCallback(char *data, uint16_t len) {
  Serial.print(F("--> Mensaje recibido desde Adafruit: "));
  Serial.println(data);

  if (strcmp(data, "ON") == 0) {
    digitalWrite(2, HIGH);
    boton = true;
  } else if (strcmp(data, "OFF") == 0) {
    digitalWrite(2, LOW);
    boton = false;
  }

  actualizarLED();
}

// Función de conexión MQTT
void MQTT_connect() {
  int8_t ret;
  if (mqtt.connected()) {
    return;
  }

  Serial.print(F("Conectando a MQTT... "));
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println(F("Reintentando conexión en 5 segundos..."));
    mqtt.disconnect();
    delay(5000);
  }
  Serial.println(F("¡MQTT Conectado!"));
}

void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode(2, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_O, OUTPUT);
  pinMode(PIN_Y, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_R, OUTPUT);

  Serial.println(F("Conectando a Wi-Fi..."));
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado.");

  sub_feed.setCallback(interruptorCallback);
  mqtt.subscribe(&sub_feed);
}

void loop() {
  MQTT_connect();

  // Leemos la temperatura con la función
  temperaturaCelsius = leerTemperaturaCelsius();

  // Actualizamos los LEDs con la nueva lectura
  actualizarLED();

  // --- BLOQUE DE PUBLICACIÓN ---
  Serial.print(F("<-- Publicando temperatura en Adafruit... "));
  Serial.print(temperaturaCelsius);

  if (!pub_feed.publish(temperaturaCelsius)) {
    Serial.println(F(" ¡Falló la publicación!"));
  } else {
    Serial.println(F(" ¡Publicado con éxito!"));
  }

  // --- RETRASO Y ESCUCHA SIMULTÁNEA ---
  mqtt.processPackets(5000);

  if (!mqtt.ping()) {
    mqtt.disconnect();
  }
}
