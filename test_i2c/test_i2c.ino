#include <Arduino.h>
#include <Wire.h>

#define SLAVE_ADDRESS 0x08  // Indirizzo I2C dello slave

void setup() {
  Wire.setSDA(4);
  Wire.setSCL(5);
  Wire.begin(SLAVE_ADDRESS);
  
  Wire.onReceive(receiveData); // Imposta la funzione di callback per la ricezione dei dati
  Serial.begin(9600);  // Inizializza la comunicazione seriale per il debug
}

void LED_Control(bool* LED_ON_OFF) {
  if (digitalRead(LED_BUILTIN) == HIGH) {
    digitalWrite(LED_BUILTIN, LOW);
    *LED_ON_OFF = false;
  } else if (digitalRead(LED_BUILTIN) == LOW) {
    digitalWrite(LED_BUILTIN, HIGH);
    *LED_ON_OFF = true;
  }
}

bool LED_ON_OFF = false;

void loop() {
  LED_Control(&LED_ON_OFF);
  delay(500);
  // Il loop viene eseguito continuamente, ma in questo esempio non viene utilizzato
}

void receiveData(int byteCount) {
  while (Wire.available()) {
    char receivedData = Wire.read();  // Leggi il dato ricevuto
    
    // Fai qualcosa con il dato ricevuto
    // In questo esempio, stampiamo il dato sulla porta seriale
    Serial.print("Dato ricevuto: ");
    Serial.println(receivedData);
  }
}
