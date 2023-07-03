//Librerie Arduino
#include <Arduino.h>
#include <Wire.h>
#include <ArduinoJson.h>

#include "sensors\Adafruit_BMP280_sensor.hpp"
#include "sensors\ArtronShop_BH1750_sensor.hpp"
#include "sensors\RTC_PCF8523_sensor.hpp"

//Librerie FreeRTOS
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

//Librerie Sensori
#include <ArtronShop_BH1750.h>
#include "RTClib.h"
#include <Adafruit_BMP280.h>


#define SLAVE_ADDRESS 0x08 // Indirizzo della Pico in modalità Slave sull'I2C0

//Definizione sensori
ArtronShop_BH1750 bh1750(0x23, &Wire1); // Non Jump ADDR: 0x23, Jump ADDR: 0x5C
Adafruit_BMP280 bmp(&Wire1); // I2C1
RTC_PCF8523 rtc;// I2C1 set in setup()

//Variabili globali dei dati ottenuti dai sensori
String BMP280_data = "";
float BH1750_data = 0.0;
unsigned long PCF8523_data = 0;

//Stato sensori
bool RTC_PCF8523_status = false;
bool Adafruit_BMP280_status = false;
bool ArtronShop_BH1750_status = false;

//Variabile globale del comando ottenuti dalla Raspberry Pi
String Received_Comand = "";
String Requested_Data = "";

//Variabili globali per monitoraggio dei sensori
#define ERROR_SENSOR_SETUP_CODE -99999
String sensor_monitor_msg = "";

//Semafori delle variabili globali dei dati dei sensori
/*
Le variabili globali saranno scritte dai Task dei singoli sensori e saranno lette dal Task che gestisce l'I2C Slave
Per cui necessita dei semafori
*/
SemaphoreHandle_t BMP280_data_Mutex;
SemaphoreHandle_t BH1750_data_Mutex;
SemaphoreHandle_t PCF8523_data_Mutex;
SemaphoreHandle_t I2C1_Mutex;

// Prototipi delle funzioni
void BMP280(void *params);
void BH1750(void *params);
void PCF8523(void *params);
void I2C_Slave_Client(void *params);

//Monitor per test
void Tasks_Monitor(void *params);

bool BMP280_setup()
{
  unsigned status;
  //status = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);
  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    //while (1) delay(10);
    return false;
  }else{
    Adafruit_BMP280_status = true;
    /* Default settings from datasheet. */
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL, /* Operating Mode. */
                Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  }
  return true;
}

bool PCF8523_setup()
{
  if (! rtc.begin(&Wire1)) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    return false;
    //while (1) delay(10);
  }else{
    RTC_PCF8523_status == true;
    if (! rtc.initialized() || rtc.lostPower())
    {
      Serial.println("RTC is NOT initialized, let's set the time!");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      
      rtc.start();
      float drift = 43; // seconds plus or minus over oservation period - set to 0 to cancel previous calibration.
      float period_sec = (7 * 86400);  // total obsevation period in seconds (86400 = seconds in 1 day:  7 days = (7 * 86400) seconds )
      float deviation_ppm = (drift / period_sec * 1000000); //  deviation in parts per million (μs)
      float drift_unit = 4.34; // use with offset mode PCF8523_TwoHours
      // float drift_unit = 4.069; //For corrections every min the drift_unit is 4.069 ppm (use with offset mode PCF8523_OneMinute)
      int offset = round(deviation_ppm / drift_unit);
      Serial.print("Offset is "); Serial.println(offset); // Print to control offset
    }
  }
  return true;
}

bool BH1750_setup()
{
  if (!bh1750.begin()) {
    Serial.println("BH1750 not found !");
    //delay(1000);
    return false;
  }else{
    ArtronShop_BH1750_status = true;
  }
  return true;
}

void setup() {
  Serial.begin(115200);
  //Definizione PIN ed inizializzazione I2C0 in Slave
  Wire.setSDA(4);
  Wire.setSCL(5);
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);

  //Definizione PIN ed inizializzazione I2C1 in Master
  Wire1.setSDA(6);
  Wire1.setSCL(7);
  Wire1.begin();

  //Creazione Task BMP280
  xTaskCreate(
  BMP280,  //void da eseguire 
  "BMP280",   // nome processo
  128,        // dimensione stack (byte)
  NULL,       // parametri
  1,          // priorità (più è alto, maggiore è la priorità)
  NULL        // riferimento per accedere al task dall'esterno
  );

  //Creazione Task BH1750
  xTaskCreate(
  BH1750,  //void da eseguire 
  "BH1750",   // nome processo
  128,        // dimensione stack (byte)
  NULL,       // parametri
  1,          // priorità (più è alto, maggiore è la priorità)
  NULL        // riferimento per accedere al task dall'esterno
  );


  //Creazione Task PCF8523
  xTaskCreate(
  PCF8523,    //void da eseguire 
  "PCF8523",  // nome processo
  128,        // dimensione stack (byte)
  NULL,       // parametri
  1,          // priorità (più è alto, maggiore è la priorità)
  NULL        // riferimento per accedere al task dall'esterno
  );

  //Creazione Task I2C_Slave_Client
  xTaskCreate(
  I2C_Slave_Client,     //void da eseguire 
  "I2C_Slave_Client",   // nome processo
  128,                  // dimensione stack (byte)
  NULL,                 // parametri
  1,                    // priorità (più è alto, maggiore è la priorità)
  NULL                  // riferimento per accedere al task dall'esterno
  );

  //Creazione Task PCF8523
  xTaskCreate(
  Tasks_Monitor,    //void da eseguire 
  "Tasks_Monitor",  // nome processo
  128,        // dimensione stack (byte)
  NULL,       // parametri
  1,          // priorità (più è alto, maggiore è la priorità)
  NULL        // riferimento per accedere al task dall'esterno
  );

  //Avvio scheduler di FreeRTOS
  vTaskStartScheduler();

  I2C1_Mutex = xSemaphoreCreateMutex();
  /* Non necessari con FreeRTOS (ogni task ha il suo setup())
  BH1750_setup();   //Setup sensore BH1750
  PCF8523_setup();  //Setup sensore PCF8523
  BMP280_setup();   //Setup sensore BMP280
  */

}

//Task BMP280
void BMP280(void *params){
  /*setup()*/
  if(xSemaphoreTake(I2C1_Mutex, portMAX_DELAY) == pdTRUE)
    {
      while(!PCF8523_setup()) /*tenta il setup del sensore finchè non va a buon fine*/
      {
        PCF8523_data = ERROR_SENSOR_SETUP_CODE;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
      }
    }
    xSemaphoreGive(I2C1_Mutex);

  /*loop()*/
  while (true)
  {
    //BMP280_data_read(); //da decommentare quando avremo i sensori
    if(xSemaphoreTake(I2C1_Mutex, portMAX_DELAY) == pdTRUE)
    {
      if (xSemaphoreTake(BMP280_data_Mutex, portMAX_DELAY) == pdTRUE)
        {
          BMP280_data = String(random(0,50)) + "," + String(random(500, 2000)) + "," + String(random(0, 10000));
          xSemaphoreGive(BMP280_data_Mutex);
        }
      xSemaphoreGive(I2C1_Mutex);
  }
  vTaskDelay(1000 / portTICK_PERIOD_MS); //delay gestito da FreeRTOS
}


}

//Task BH1750
void BH1750(void *params){
  /*setup()*/
  if(xSemaphoreTake(I2C1_Mutex, portMAX_DELAY) == pdTRUE)
    {
      while (!BH1750_setup) /*tenta il setup del sensore finchè non va a buon fine*/
      {
        BH1750_data = ERROR_SENSOR_SETUP_CODE;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
      }
    }
    xSemaphoreGive(I2C1_Mutex);

  //BH1750_setup(); //da decommentare quando avremo i sensori

  /*loop()*/
  while (true)
  {
    if(xSemaphoreTake(I2C1_Mutex, portMAX_DELAY) == pdTRUE)
    {
      if (xSemaphoreTake(BH1750_data_Mutex, portMAX_DELAY) == pdTRUE)
      {
        BH1750_data = BH1750_data_read();//da decommentare quando avremo i sensori
        //BH1750_data =  random(0.0,1.1);
        xSemaphoreGive(BH1750_data_Mutex);
      }
      xSemaphoreGive(I2C1_Mutex);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS); //delay gestito da FreeRTOS
  }
}

//Task PCF8523
void PCF8523(void *params){
  /*setup()*/
    if(xSemaphoreTake(I2C1_Mutex, portMAX_DELAY) == pdTRUE)
    {
      while(!PCF8523_setup()) /*tenta il setup del sensore finchè non va a buon fine*/
      {
        PCF8523_data = ERROR_SENSOR_SETUP_CODE;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
      }
    }
    xSemaphoreGive(I2C1_Mutex);

  /*loop()*/
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

void Tasks_Monitor(void *params){
/*setup()*/
  String BMP280_aux = "";
  float BH1750_aux = 0.0;
  unsigned long PCF8523_aux = 0;

/*loop()*/
  while (true)
  {
    if (xSemaphoreTake(PCF8523_data_Mutex, portMAX_DELAY) == pdTRUE)
    {
      PCF8523_aux =  PCF8523_data;
      xSemaphoreGive(PCF8523_data_Mutex);
    }

    if (xSemaphoreTake(BH1750_data_Mutex, portMAX_DELAY) == pdTRUE)
    {
      BH1750_aux =  BH1750_data;
      xSemaphoreGive(BH1750_data_Mutex);
    }

    if (xSemaphoreTake(BMP280_data_Mutex, portMAX_DELAY) == pdTRUE)
    {
      BMP280_aux =  BMP280_data;
      xSemaphoreGive(BMP280_data_Mutex);
    }
    String sensor_monitor_msg_aux = "|PCF8523: " + String(PCF8523_aux) + " |BH1750: " + String(BH1750_aux) + " |BMP280: " + String(BMP280_aux);
    
    if(sensor_monitor_msg != sensor_monitor_msg_aux)
    {/*Il monitor stampa il i valori dei sensori solo se diversi dalp print precedente*/
      sensor_monitor_msg = sensor_monitor_msg_aux;
      Serial.println(sensor_monitor_msg);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS); //delay gestito da FreeRTOS
  }
}

//CallBack, parte in automatico quando ci sono richieste sul'I2C0
void receiveData(int byteCount) {
  while (Wire.available()) {
    char data = Wire.read();
    // Elabora i dati ricevuti come necessario
    Serial.print("Comando ricevuto: ");
    Serial.println(data);
    Received_Comand += data;
  }
}

void sendData() {
  for (size_t i = 0; i < Requested_Data.length(); i++) {
    Wire.write(Requested_Data[i]);
  Requested_Data = "";
  }
}

String generateJsonString(String name, String data, String timestamp) {

  StaticJsonDocument<200> doc;
  doc["name"] = name;
  doc["data"] = data;
  doc["timestamp"] = timestamp;

  String jsonString;
  serializeJson(doc, jsonString);

  return jsonString;
}

void I2C_Slave_Client(void *params){
/*setup()*/
 
/*loop()*/
  while (true){
    if(Received_Comand == "0")
    {
      xSemaphoreTake(BH1750_data_Mutex, portMAX_DELAY);
      String jsonString = generateJsonString("BH1750", String(BH1750_data), String(millis()));
      Requested_Data = jsonString;
      //Requested_Data = String("BH1750" + String(BH1750_data) + "," + String(millis()));
      xSemaphoreGive(BH1750_data_Mutex);

    }else if(Received_Comand == "1")
    {
      xSemaphoreTake(BMP280_data_Mutex, portMAX_DELAY);
      String jsonString = generateJsonString("BMP280", String(BMP280_data), String(millis()));
      Requested_Data = jsonString;
      //Requested_Data = String("BH1750" + String(BH1750_data) + "," + String(millis()));
      xSemaphoreGive(BMP280_data_Mutex);

    }else if(Received_Comand == "2")
    {
      xSemaphoreTake(PCF8523_data_Mutex, portMAX_DELAY);
      String jsonString = generateJsonString("PCF8523", String(PCF8523_data), String(millis()));
      Requested_Data = jsonString;
      //Requested_Data = String("PCF8523" + String(PCF8523_data) + "," + String(millis()));
      xSemaphoreGive(PCF8523_data_Mutex); 

    }
  }
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

void loop(){}
/*
  void loop() //SOLO PER TEST, CON FREERTOS QUESTO VA VIA
{
/*
  String alt_value = BMP280_data_read();
  float lux = BH1750_data_read();
  int timestamp = millis();
*/
/*
  String alt_value = String(random(0,50)) + "," + String(random(500, 2000)) + "," + String(random(0, 10000)); //random(min, max)
  float lux = random(0.0,1.1);
  int timestamp = millis();


  String sensors_test = String("alt_value: " + alt_value + "| lux: " + String(lux) + "| TimeStamp: " + String(timestamp));

}
*/



