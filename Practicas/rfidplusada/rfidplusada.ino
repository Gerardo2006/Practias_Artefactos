//Alimentación: VCC:3.3 V (si lo conectas a 5V puedes quemar el módulo RFID).n GND: GND.
//Pines del bus SPI (asignados por defecto en el ESP32 al llamar SPI.begin()): SCK: GPIO 18 MOSI: GPIO 23 MISO: GPIO 19
//Pines definidos en el código: SDA (SS): GPIO 5 (#define SS_PIN 5) RST: GPIO 22 (#define RST_PIN 22)

#include <WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <SPI.h>
#include <MFRC522.h>

// Credenciales Wi-Fi y Adafruit IO
#define WLAN_SSID       "Cornejo_2.4G"
#define WLAN_PASS       "papaluis"
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME  "Gerardo2006"
#define AIO_KEY       "aio_bVze79OApfXqlGEC0qr39UjKBOxP"

// Conexiones RFID MFRC522 (SPI)
#define RST_PIN 22
#define SS_PIN  5

MFRC522 rfid(SS_PIN, RST_PIN);
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Feed de publicación para el UID de la tarjeta en Adafruit IO
Adafruit_MQTT_Publish feedRFID = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/rfid");

void MQTT_connect();

void setup() {
  Serial.begin(115200);
  while(!Serial); // Esperar comunicación serial si es necesario

  SPI.begin();
  rfid.PCD_Init();
  Serial.println("Acerca una tarjeta RFID al lector...");

  // Conexión a la red Wi-Fi
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Conectado!");
}

void loop() {
  MQTT_connect();
  mqtt.processPackets(100);

  // Verificar si hay una nueva tarjeta presente
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  // Verificar si se puede leer la tarjeta
  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Imprimir UID en el Monitor Serial y construir cadena para Adafruit IO
  String uidString = "";
  Serial.print("UID de la tarjeta: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);

    // Cadena hex para publicar a la nube
    if (rfid.uid.uidByte[i] < 0x10) uidString += "0";
    uidString += String(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();
  uidString.toUpperCase();

  // Publicar el UID a Adafruit IO
  feedRFID.publish(uidString.c_str());

  // Detener la comunicación con la tarjeta actual
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(1000); // Pausa pequeña para evitar lecturas duplicadas inmediatas
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