int sensorLDR; 
float voltaje; 

// Pines seguros de salida para el LED RGB 
const int ledR = 25; // Pin Rojo (GPIO25) 
const int ledG = 32; // Pin Verde (GPIO32) 
const int ledB = 33; // Pin Azul (GPIO33) 
const int pinLDR = 34; // Entrada analógica de la LDR (GPIO34) 

void setup() { 
  Serial.begin(115200); 
  pinMode(ledR, OUTPUT); 
  pinMode(ledG, OUTPUT); 
  pinMode(ledB, OUTPUT); 
} 

// Función auxiliar para ajustar el color mediante PWM (0 a 255) 
void setColor(int red, int green, int blue) { 
  analogWrite(ledR, red); 
  analogWrite(ledG, green); 
  analogWrite(ledB, blue); 
} 

void loop() { 
  // Lectura de la LDR (0 a 4095 en la resolución de 12 bits del ESP32) 
  sensorLDR = analogRead(pinLDR); 
  
  // Cálculo del voltaje correspondiente (0 a 3.3V) 
  voltaje = (sensorLDR * 3.3) / 4095.0; 
  
  // Imprimir valores en el Monitor Serie 
  Serial.print("LDR (ADC): "); 
  Serial.print(sensorLDR); 
  Serial.print(" | Voltaje: "); 
  Serial.print(voltaje); 
  Serial.println(" V"); 
  
  // Variación de colores según la luz que recibe la LDR 
  if (sensorLDR < 800) { 
    setColor(255, 0, 0); // Oscuro / Poca luz: Rojo 
  } else if (sensorLDR >= 800 && sensorLDR < 1600) { 
    setColor(255, 128, 0); // Luz baja: Naranja 
  } else if (sensorLDR >= 1600 && sensorLDR < 2400) { 
    setColor(128, 0, 128); // Luz media: Morado 
  } else if (sensorLDR >= 2400 && sensorLDR < 3200) { 
    setColor(0, 0, 255); // Luz ambiente alta: Azul 
  } else { 
    setColor(0, 255, 0); // Mucha luz (linterna): Verde 
  } 
  
  delay(200); 
}