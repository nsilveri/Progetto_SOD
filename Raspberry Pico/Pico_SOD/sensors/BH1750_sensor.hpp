#ifndef BH1750_CLASS_H
#define BH1750_CLASS_H

//Librerie Arduino
#include <Arduino.h>
#include <Wire.h>
#define BH1750_ADDRESS 0x23

//Librerie FreeRTOS
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

//Librerie Sensori
#include <BH1750_WE.h>

// Inizializzazione del sensore su bus I2C1
BH1750_WE myBH1750 = BH1750_WE(&Wire1, BH1750_ADDRESS);

//Setup del sensore
void BH1750_setup()
{
  while(!myBH1750.init()){ 
    Serial.println("Connection to the BH1750 failed");
    Serial.println("Check wiring and I2C address");
  }
    Serial.println("BH1750 is connected");
}

// Lettura del dato del sensore
float BH1750_data_read() 
{
  float lightIntensity = myBH1750.getLux();
  return lightIntensity;
}

#endif