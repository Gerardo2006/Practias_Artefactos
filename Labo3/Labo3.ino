#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define i2c_Address 0x3C

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

int entrada_sensor;
float salida_sensor;

Adafruit_SH1106G display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

void setup() {
  Serial.begin(9600);
  
  Wire.begin();
  delay(250);

  if (!display.begin(i2c_Address, true)) {
    Serial.println("ERROR: OLED no encontrada");
    while (1);
  }

  Serial.println("OLED funcionando correctamente");
}

void loop() {
  entrada_sensor = analogRead(A0);
  
  salida_sensor = (entrada_sensor * (5.0 / 1024.0)) * 100.0;

  Serial.print("Temperatura actual: ");
  Serial.print(salida_sensor);
  Serial.println(" C");

  display.clearDisplay();

  display.setTextColor(SH110X_WHITE);
  display.setTextSize(2);
  display.setCursor(15, 5);
  display.println("Salida:");

  display.setTextSize(2);
  display.setCursor(15, 35);
  display.print(salida_sensor, 1);
  display.println("° C");

  display.display();

  delay(1000); 
}