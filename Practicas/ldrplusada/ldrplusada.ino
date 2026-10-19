//Función: Lee el nivel de luz (ADC 12 bits: 0 a 4095) y publica la lectura o porcentaje en el feed luz
//Conexiones: Divisor de tensión con resistencia de 10 kΩ al pin analógico GPIO 34 y voltaje a 3.3V

#include <WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

#define WLAN_SSID       "Cornejo_2.4G"
#define WLAN_PASS       "papaluis"
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME  "Gerardo2006"
#define AIO_KEY       "aio_bVze79OApfXqlGEC0qr39UjKBOxP"

#define LDR_PIN 34

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish feedLDR = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/LDR");

void MQTT_connect();

void setup() {
  Serial.begin(115200);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Conectado");
}

void loop() {
  MQTT_connect();

  int lecturaADC = analogRead(LDR_PIN);
  float porcentajeLuz = (lecturaADC / 4095.0) * 100.0;

  Serial.print("Lectura LDR: ");
  Serial.print(lecturaADC);
  Serial.print(" | Luz: ");
  Serial.print(porcentajeLuz);
  Serial.println("%");

  feedLDR.publish(porcentajeLuz);

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