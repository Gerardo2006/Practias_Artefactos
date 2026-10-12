const int pinSensor = 35; 
float voltaje;
float temperatura;

void setup() {
  Serial.begin(115200); 
  analogReadResolution(12);          
  analogSetAttenuation(ADC_11db);
}

void loop() {
  long sumaValores = 0;
  int muestras = 20;

  for (int i = 0; i < muestras; i++) {
    sumaValores += analogRead(pinSensor);
    delay(10); 
  }
  
  int lecturaPromedio = sumaValores / muestras;
  

  voltaje = (lecturaPromedio * 3300) / 4095;
  
  temperatura = voltaje / 0.01; 

  Serial.print("Voltaje leido: ");
  Serial.print(voltaje, 3);
  Serial.print(" V | Temperatura: ");
  Serial.print(temperatura, 1);
  Serial.println(" C");

  delay(1000); 
}