#ifndef PCF8523_CLASS_H
#define PCF8523_CLASS_H

//Librerie Arduino
#include <Arduino.h>
#include <Wire.h>

//Librerie Sensori
#include "RTClib.h"

RTC_PCF8523 rtc;  // I2C1 set in setup()

unsigned long PCF8523_data = 0;

//Stato sensori
bool RTC_PCF8523_status = false;

bool PCF8523_setup()
{
  if (! rtc.begin(&Wire1)) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

float PCF8523_data_read(){

  DateTime now = rtc.now();
  String ts = String(now);
  int timestamp = ts; //millis();
  return timestamp;
    
}

#endif