//Función: Lee humedad y temperatura y las publica a los feeds temperatura y humedad
//Librerías requeridas: DHT sensor library y Adafruit Unified Sensor
//Conexiones: VCC -> 3.3V, GND -> GND, Data -> GPIO 4

#include <WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include "DHT.h"

#define WLAN_SSID       "Cornejo_2.4G"
#define WLAN_PASS       "papaluis"
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME  "Gerardo2006"
#define AIO_KEY       "aio_bVze79OApfXqlGEC0qr39UjKBOxP"

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish feedTemp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperatura");
Adafruit_MQTT_Publish feedHum = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humedad");

void MQTT_connect();

void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Conectado");
}

void loop() {
  MQTT_connect();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    Serial.print("Temp: ");
    Serial.print(t);
    Serial.print(" C | Humedad: ");
    Serial.print(h);
    Serial.println(" %");

    feedTemp.publish(t);
    feedHum.publish(h);
  } else {
    Serial.println("Error al leer el sensor DHT11");
  }

  mqtt.processPackets(5000);
  if (!mqtt.ping()) {
    mqtt.disconnect();
  }
}

void MQTT_connect() {
  int8_t ret;
  if (mqtt.connected()) return;

  Serial.print("Conectando a MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Reintentando conexion en 5 segundos...");
    mqtt.disconnect();
    delay(5000);
    retries--;
    if (retries == 0) {
      while (1);
    }
  }
  Serial.println("MQTT Conectado!");
}