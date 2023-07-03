#ifndef BMP280_CLASS_H
#define BMP280_CLASS_H

//Librerie Arduino
#include <Arduino.h>
#include <Wire.h>

//Librerie Sensori
#include <Adafruit_BMP280.h>

#define ERROR_SENSOR_SETUP_CODE -99999

Adafruit_BMP280 bmp(&Wire1); // I2C1

//String BMP280_data = "";

bool Adafruit_BMP280_status = false;
#define ERROR_SENSOR_SETUP_CODE -99999

void BMP280_setup()
{
  while (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    //delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

String BMP280_data_read()
{

 float temp = bmp.readTemperature();
 float bar =  bmp.readPressure();
 float alt =  bmp.readAltitude();

 String sensor_data = "T: " + String(temp) + "*C, P: " + String(bar) + " Pa, H: " + String(alt) + " m";

 return sensor_data;
}

/*
  //Task BMP280
void BMP280(void *params){
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
      if (xSemaphoreTake(BMP280_data_Mutex, portMAX_DELAY) == pdTRUE)
        {
          BMP280_data = BMP280_data_read();
          xSemaphoreGive(BMP280_data_Mutex);
        }
      xSemaphoreGive(I2C1_Mutex);
  }
  vTaskDelay(1000 / portTICK_PERIOD_MS); //delay gestito da FreeRTOS
}
*/

#endif