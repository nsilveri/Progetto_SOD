#ifndef PCF8523_CLASS_H
#define PCF8523_CLASS_H

//Librerie Arduino
#include <Arduino.h>
#include <Wire.h>

//Librerie Sensori
#include "RTClib.h"

RTC_PCF8523 rtc;  // I2C1 set in setup()

//unsigned long PCF8523_data = 0;

//Stato sensori
bool RTC_PCF8523_status = false;

void RTC_setup()
{
  Serial.println("1");
  if (! rtc.begin(&Wire1)) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
  }
  Serial.println("2");
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
Serial.println("3");
}

String RTC_data_read(){

  DateTime now = rtc.now();
  char buf2[] = "YYMMDD-hh:mm:ss";
  String ts = now.toString(buf2);
  return ts;
}

#endif