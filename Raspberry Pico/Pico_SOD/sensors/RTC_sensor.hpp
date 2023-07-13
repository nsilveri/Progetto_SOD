#ifndef PCF8523_CLASS_H
#define PCF8523_CLASS_H

//Librerie Arduino
#include <Arduino.h>
#include <Wire.h>

//Librerie Sensori
#include "RTClib.h"

bool RTC_LOG = false;

RTC_DS1307 rtc;  // I2C1 set in setup()

//Stato sensori
bool RTC_PCF8523_status = false;

void RTC_setup()
{
  if (! rtc.begin(&Wire1)) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
  }
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

String RTC_data_read_old(){

  DateTime now = rtc.now();
  char buf2[] = "YYMMDD-hh:mm:ss";
  String ts = now.toString(buf2);
  return ts;
}

int RTC_data_read()
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
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");
  }

  int TS =  now.unixtime();

  return TS;
}

#endif