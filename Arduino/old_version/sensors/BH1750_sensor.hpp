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

BH1750_WE myBH1750 = BH1750_WE(&Wire1, BH1750_ADDRESS);

#define ERROR_SENSOR_SETUP_CODE -99999

//Variabili globali dei dati ottenuti dai sensori
//float BH1750_data = 0.0;

ArtronShop_BH1750 bh1750(0x23, &Wire1);

SemaphoreHandle_t BH1750_data_Mutex;
SemaphoreHandle_t I2C1_Mutex;

bool ArtronShop_BH1750_status = false;

// Prototipi delle funzioni
void BH1750(void *params);

void BH1750_setup()
{
  while(!myBH1750.init()){ // sets default values: mode = CHM, measuring time factor = 1.0
    Serial.println("Connection to the BH1750 failed");
    Serial.println("Check wiring and I2C address");
    //while(1){}
  }
    Serial.println("BH1750 is connected");
}

float BH1750_data_read() 
{
  float lightIntensity = myBH1750.getLux();
  Serial.print(F("Light intensity: "));
  Serial.print(lightIntensity);
  Serial.println(F(" Lux"));
  //delay(1000);
}

/*
//Task PCF8523
void PCF8523(void *params){
    if(xSemaphoreTake(I2C1_Mutex, portMAX_DELAY) == pdTRUE)
    {
      while(!PCF8523_setup()) 
      {
        PCF8523_data = ERROR_SENSOR_SETUP_CODE;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
      }
    }
    xSemaphoreGive(I2C1_Mutex);

  while (true)
  {
    if(xSemaphoreTake(I2C1_Mutex, portMAX_DELAY) == pdTRUE)
    {
      if (xSemaphoreTake(PCF8523_data_Mutex, portMAX_DELAY) == pdTRUE)
      {
        PCF8523_data = PCF8523_data_read(); //da decommentare quando avremo i sensori
        //PCF8523_data = millis();
        xSemaphoreGive(PCF8523_data_Mutex);
      }
      xSemaphoreGive(I2C1_Mutex);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS); //delay gestito da FreeRTOS
  }
}
*/


#endif