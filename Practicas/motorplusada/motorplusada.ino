//Función: Se suscribe al feed servo para recibir un ángulo (0 a 180°) mediante un slider en el Dashboard de Adafruit IO.
//Librería requerida: ESP32Servo.
//Conexiones: Señal -> GPIO 13, VCC -> 5V/VIN, GND -> GND.
#include <WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <ESP32Servo.h>

// Credenciales Wi-Fi y Adafruit IO
#define WLAN_SSID       "Cornejo_2.4G"
#define WLAN_PASS       "papaluis"
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME  "Gerardo2006"
#define AIO_KEY       "aio_bVze79OApfXqlGEC0qr39UjKBOxP"

#define SERVO_PIN 26

Servo servo;
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Feed de Suscripción en Adafruit IO
Adafruit_MQTT_Subscribe feedServo = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/servo");

void MQTT_connect();

void setup() {
  Serial.begin(115200);

  // Configuración según ESP32Servo: Pin GPIO 26, min 500us, max 2500us
  servo.attach(SERVO_PIN, 500, 2500);
  servo.write(0); // Posición inicial 0 grados

  // Conexión Wi-Fi
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Conectado!");

  // Suscribirse al feed del servo
  mqtt.subscribe(&feedServo);
}

void loop() {
  MQTT_connect();

  // Escuchar mensajes entrantes desde Adafruit IO
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &feedServo) {
      char *data = (char *)feedServo.lastread;
      int angulo = atoi(data); // Convertir texto del feed a entero
      
      if (angulo >= 0 && angulo <= 180) {
        Serial.print("Moviendo Servo a: ");
        Serial.print(angulo);
        Serial.println("°");
        
        servo.write(angulo);
      }
    }
  }

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