#ifndef PCF8523_CLASS_H
#define PCF8523_CLASS_H

//Librerie Arduino
#include <Arduino.h>
#include <Wire.h>

//Librerie Sensori
#include "RTClib.h"

bool RTC_LOG = false;

// Inizializzazione del sensore
RTC_DS1307 rtc;

//Setup dell'RTC
void RTC_setup()
{
  if (! rtc.begin(&Wire1)) { // RTC inizializzato su bus I2C1
    Serial.println("Couldn't find RTC");
    Serial.flush();
  }
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

// Lettura del timestamp
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