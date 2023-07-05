#ifndef PCF8523_CLASS_H
#define PCF8523_CLASS_H

//Librerie Arduino
#include <Arduino.h>
#include <Wire.h>

//Librerie Sensori
#include "RTClib.h"

bool RTC_LOG = false;

RTC_DS1307 rtc;  // I2C1 set in setup()
//RTC_PCF8523 rtc;

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

String RTC_data_read_old(){

  DateTime now = rtc.now();
  char buf2[] = "YYMMDD-hh:mm:ss";
  String ts = now.toString(buf2);
  return ts;
}

String RTC_data_read()
{
  DateTime now = rtc.now();

  if(RTC_LOG == true)
  {
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print('/');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    Serial.print(" since midnight 1/1/1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");
  }

  uint32_t TS =  now.unixtime();

   return String(TS);
}

#endif