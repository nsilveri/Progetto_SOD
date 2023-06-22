#include <Arduino.h>
#include <Wire.h>
#include <FreeRTOS.h>
#include <ArtronShop_BH1750.h>
#include "RTClib.h"
#include <Adafruit_BMP280.h>


//INCLUDERE FREERTOS==================================================================================

ArtronShop_BH1750 bh1750(0x23, &Wire1); // Non Jump ADDR: 0x23, Jump ADDR: 0x5C
Adafruit_BMP280 bmp(&Wire1); // I2C1
RTC_PCF8523 rtc;// I2C1 set in setup()

void BMP280_setup()
{
  
  while ( !Serial ) delay(100);   // wait for native usb
  Serial.println(F("BMP280 test"));
  unsigned status;
  //status = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);
  status = bmp.begin();
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

void PCF8523_setup()
{
  if (! rtc.begin(&Wire1)) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (! rtc.initialized() || rtc.lostPower()) {
    Serial.println("RTC is NOT initialized, let's set the time!");

    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  rtc.start();
  float drift = 43; // seconds plus or minus over oservation period - set to 0 to cancel previous calibration.
  float period_sec = (7 * 86400);  // total obsevation period in seconds (86400 = seconds in 1 day:  7 days = (7 * 86400) seconds )
  float deviation_ppm = (drift / period_sec * 1000000); //  deviation in parts per million (μs)
  float drift_unit = 4.34; // use with offset mode PCF8523_TwoHours
  // float drift_unit = 4.069; //For corrections every min the drift_unit is 4.069 ppm (use with offset mode PCF8523_OneMinute)
  int offset = round(deviation_ppm / drift_unit);

  Serial.print("Offset is "); Serial.println(offset); // Print to control offset

}

void BH1750_setup()
{
    if (!bh1750.begin()) {
      Serial.println("BH1750 not found !");
      delay(1000);
  }
}

void setup() {
  Serial.begin(115200);
  //Definizione PIN ed inizializzazione I2C0
  Wire.setSDA(4);
  Wire.setSCL(5);
  Wire.begin();

  //Definizione PIN ed inizializzazione I2C1
  Wire1.setSDA(6);
  Wire1.setSCL(7);
  Wire1.begin();
  /*
  BH1750_setup();   //Setup sensore BH1750
  PCF8523_setup();  //Setup sensore PCF8523
  BMP280_setup();   //Setup sensore BMP280
  */

}



float PCF8523_data_read(){

  DateTime now = rtc.now();
  //String ts = String(now);
  int timestamp = millis();
  return timestamp;
    
}

float BH1750_data_read() 
{
  float lux_level = bh1750.light();
  return lux_level;
}

String BMP280_data_read()
{

 float temp = bmp.readTemperature();
 float bar =  bmp.readPressure();
 float alt =  bmp.readAltitude();

 String sensor_data = String(temp) + "," + String(bar) + "," + String(alt);

 return sensor_data;
}

void loop() //SOLO PER TEST, CON FREERTOS QUESTO VA VIA
{
/*
  String alt_value = BMP280_data_read();
  float lux = BH1750_data_read();
  int timestamp = millis();
*/
  String alt_value = String(random(0,50)) + "," + String(random(500, 2000)) + "," + String(random(0, 10000)); //random(min, max)
  float lux = random(0.0,1.1);
  int timestamp = millis();


  String sensors_test = String("alt_value: " + alt_value + "| lux: " + String(lux) + "| TimeStamp: " + String(timestamp));

}
