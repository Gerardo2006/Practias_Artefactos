//Función: Mide distancia en cm y la publica en el feed distancia
//Conexiones: VCC -> 5V/VIN, GND -> GND, Trig -> GPIO 18, Echo -> GPIO 193

#include <WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <SPI.h>
#include <MFRC522.h>

#define WLAN_SSID       "Cornejo_2.4G"
#define WLAN_PASS       "papaluis"
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME  "Gerardo2006"
#define AIO_KEY       "aio_bVze79OApfXqlGEC0qr39UjKBOxP"

#define TRIG_PIN 18
#define ECHO_PIN 19

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish feedultraSonido = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/ultraSonido");

void MQTT_connect();

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Conectado");
}

void loop() {
  MQTT_connect();

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duracion = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duracion == 0) {
    Serial.println("No se detectó eco");
  } else {
    float distancia = (0.0343 * duracion) / 2.0;

    Serial.println("El objeto esta a: ");
    Serial.println(distancia);

    // Publicación a Adafruit IO
    feedultraSonido.publish(distancia);
  }

  // Espera y procesamiento de paquetes para mantener la conexión activa con Adafruit IO
  mqtt.processPackets(3000);
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