//Arduino libraries
#include <ArduinoJson.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

/*
  #include "sensors\Adafruit_BMP280_sensor.hpp"
  #include "sensors\BH1750_sensor.hpp"
  #include "sensors\RTC_PCF8523_sensor.hpp"
*/
#include "sensors\BMP280_sensor.hpp"
#include "sensors\BH1750_sensor.hpp"
#include "sensors\RTC_sensor.hpp"

/*
#define BMP280_TASK_DELAY        150
#define BH1750_TASK_DELAY        140
#define RTC_TASK_DELAY           130
#define TASK_MONITOR_TASK_DELAY  500
*/
#define CORE_0 (1 << 0)
#define CORE_1 (1 << 1)


uint16_t BMP280_TASK_DELAY        = 150 + random(0, 20);
uint16_t BH1750_TASK_DELAY        = 150 + random(0, 20);
uint16_t RTC_TASK_DELAY           = 150 + random(0, 20);
uint16_t TASK_MONITOR_TASK_DELAY  = 500 + random(0, 20);


#define SLAVE_ADDRESS 0x08 // Indirizzo della Pico in modalità Slave sull'I2C0

/* Dimensions of the buffer that the task being created will use as its stack.
  NOTE:  This is the number of words the stack will hold, not the number of
  bytes.  For example, if each stack item is 32-bits, and this is set to 100,
  then 400 bytes (100 * 32-bits) will be allocated. */
#define STACK_SIZE 512

/* Structure that will hold the TCB of the task being created. */
StaticTask_t xTaskBuffer_BMP280;
StaticTask_t xTaskBuffer_BH1750;
StaticTask_t xTaskBuffer_RTC;
StaticTask_t xTaskBuffer_TASK_MONITOR;

/* Buffer that the task being created will use as its stack.  Note this is
  an array of StackType_t variables.  The size of StackType_t is dependent on
  the RTOS port. */
StackType_t xStack_BMP280[ STACK_SIZE ];
StackType_t xStack_BH1750[ STACK_SIZE ];
StackType_t xStack_RTC[ STACK_SIZE ];
StackType_t xStack_TASK_MONITOR[ STACK_SIZE ];

SemaphoreHandle_t xSemaphore      ;
SemaphoreHandle_t I2C1_Semaphore  ;

SemaphoreHandle_t BH1750_Semaphore;
SemaphoreHandle_t BMP280_Semaphore;
SemaphoreHandle_t RTC_Semaphore   ;

StaticSemaphore_t xMutexBuffer;

StaticSemaphore_t xI2C1_MutexBuffer;

StaticSemaphore_t xBH1750_MutexBuffer;
StaticSemaphore_t xBMP280_MutexBuffer;
StaticSemaphore_t xRTC_MutexBuffer;

QueueHandle_t data;

String Received_Comand = "";
String Requested_Data = "";

String BH1750_data  = "NULL";
String BMP280_data  = "NULL";
String TimeStamp    = "NULL";

unsigned long startTime;


void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  Wire.setSDA(4);
  Wire.setSCL(5);
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);

  //Definizione PIN ed inizializzazione I2C1 in Master
  Wire1.setSDA(2);
  Wire1.setSCL(3);
  Wire1.begin();

  delay(2000);
  Serial.println(F("Avvio...."));
  delay(500);

  Serial.println(F("Setup BMP280...."));
  BMP280_setup();
  delay(500);

  Serial.println(F("Setup BH1750...."));
  BH1750_setup();
  delay(500);
  
  Serial.println(F("Setup RTC...."));
  RTC_setup();
  delay(500);

  startTime = millis();  // Salva il tempo di avvio iniziale


  /* Create a mutex semaphore without using any dynamic memory
    allocation.  The mutex's data structures will be saved into
    the xMutexBuffer variable. */
  //xSemaphore       = xSemaphoreCreateMutexStatic( &xMutexBuffer );

  BH1750_Semaphore = xSemaphoreCreateMutexStatic( &xBH1750_MutexBuffer );
  BMP280_Semaphore = xSemaphoreCreateMutexStatic( &xBMP280_MutexBuffer );
  RTC_Semaphore    = xSemaphoreCreateMutexStatic( &xRTC_MutexBuffer );

  I2C1_Semaphore   = xSemaphoreCreateMutexStatic( &xMutexBuffer );

  /*
    xTaskCreateStatic(BMP280_Task,       "BMP280_Task",       STACK_SIZE, NULL, configMAX_PRIORITIES - 1, xStack_BMP280,       &xTaskBuffer_BMP280);
    xTaskCreateStatic(BH1750_Task,       "BH1750_Task",       STACK_SIZE, NULL, configMAX_PRIORITIES - 1, xStack_BH1750,       &xTaskBuffer_BH1750);
    xTaskCreateStatic(RTC_Task,          "RTC_Task",          STACK_SIZE, NULL, configMAX_PRIORITIES - 1, xStack_RTC,          &xTaskBuffer_RTC);
    xTaskCreateStatic(TASK_MONITOR_Task, "TASK_MONITOR_Task", STACK_SIZE, NULL, configMAX_PRIORITIES - 2, xStack_TASK_MONITOR, &xTaskBuffer_TASK_MONITOR);
  */
  
  xTaskCreateStaticAffinitySet(BMP280_Task,       "BMP280_Task",       STACK_SIZE, NULL, configMAX_PRIORITIES - 1, xStack_BMP280,       &xTaskBuffer_BMP280,       1);
  xTaskCreateStaticAffinitySet(BH1750_Task,       "BH1750_Task",       STACK_SIZE, NULL, configMAX_PRIORITIES - 1, xStack_BH1750,       &xTaskBuffer_BH1750,       1);
  xTaskCreateStaticAffinitySet(RTC_Task,          "RTC_Task",          STACK_SIZE, NULL, configMAX_PRIORITIES - 1, xStack_RTC,          &xTaskBuffer_RTC,          1);
  xTaskCreateStaticAffinitySet(TASK_MONITOR_Task, "TASK_MONITOR_Task", STACK_SIZE, NULL, configMAX_PRIORITIES - 1, xStack_TASK_MONITOR, &xTaskBuffer_TASK_MONITOR, 1);

}

void BMP280_Task(void *pvParameters)
{
  (void) pvParameters;
  /*
      SETUP HERE
      xSemaphoreTake( I2C1_Semaphore, ( TickType_t ) portMAX_DELAY );
      BMP280_setup();
      xSemaphoreGive( I2C1_Semaphore );
  */
  ////vTaskDelay(BMP280_TASK_DELAY / portTICK_PERIOD_MS);
  //Serial.println(F("11|"));

  while (1)
  {
    vTaskDelay(BMP280_TASK_DELAY / portTICK_PERIOD_MS);
    xSemaphoreTake( I2C1_Semaphore, ( TickType_t ) portMAX_DELAY );
    /*
        LOOP HERE
    */
    xSemaphoreTake( BMP280_Semaphore, ( TickType_t ) portMAX_DELAY );
    BMP280_data = BMP280_data_read();
    xSemaphoreGive( BMP280_Semaphore );

    xSemaphoreGive( I2C1_Semaphore );
  }
}

void BH1750_Task(void *pvParameters)
{
  (void) pvParameters;
  /*
      SETUP HERE
      xSemaphoreTake( I2C1_Semaphore, ( TickType_t ) portMAX_DELAY );
      BH1750_setup();
      xSemaphoreGive( I2C1_Semaphore );
  */
  ////vTaskDelay(BH1750_TASK_DELAY / portTICK_PERIOD_MS);
  

  while (1)
  {
    vTaskDelay(BH1750_TASK_DELAY / portTICK_PERIOD_MS);
    xSemaphoreTake( I2C1_Semaphore, ( TickType_t ) portMAX_DELAY );
    /*
        LOOP HERE
    */
    xSemaphoreTake( BH1750_Semaphore, ( TickType_t ) portMAX_DELAY );
    BH1750_data = BH1750_data_read();
    xSemaphoreGive( BH1750_Semaphore );

    xSemaphoreGive( I2C1_Semaphore );
  }
}

void RTC_Task(void *pvParameters)
{
  (void) pvParameters;
  /*
      SETUP HERE
      xSemaphoreTake( I2C1_Semaphore, ( TickType_t ) portMAX_DELAY );
      RTC_setup();
      xSemaphoreGive( I2C1_Semaphore );
  */
  ////vTaskDelay(RTC_TASK_DELAY / portTICK_PERIOD_MS);
  

  while (1)
  {
    vTaskDelay(RTC_TASK_DELAY / portTICK_PERIOD_MS);
    xSemaphoreTake( I2C1_Semaphore, ( TickType_t ) portMAX_DELAY );
    /*
        LOOP HERE
    */
    xSemaphoreTake( RTC_Semaphore, ( TickType_t ) portMAX_DELAY );
    TimeStamp = RTC_data_read();
    xSemaphoreGive( RTC_Semaphore );
    xSemaphoreGive( I2C1_Semaphore );
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

void sys_timer() {
  unsigned long currentTime = millis();  // Ottieni il tempo corrente
  unsigned long elapsedTime = currentTime - startTime;  // Calcola il tempo trascorso

  unsigned int seconds = elapsedTime / 1000;  // Converti il tempo in secondi

  Serial.println(seconds);  // Stampa i secondi su Serial Monitor

  delay(1000);  // Attendiamo un secondo prima di calcolare nuovamente i secondi
}

void TASK_MONITOR_Task(void *pvParameters)
{
  (void) pvParameters;
  /*
      SETUP HERE
  */
  String monitor_message = "";
  bool LED_ON_OFF = false;

  while (1)
  {
    BMP280_TASK_DELAY        = 150 + random(0, 20);
    BH1750_TASK_DELAY        = 150 + random(0, 20);
    RTC_TASK_DELAY           = 150 + random(0, 20);
    TASK_MONITOR_TASK_DELAY  = 500 + random(0, 20);
    
    vTaskDelay(TASK_MONITOR_TASK_DELAY / portTICK_PERIOD_MS);
    //xSemaphoreTake( I2C1_Semaphore, ( TickType_t ) portMAX_DELAY );
    /*
        LOOP HERE
    */
    xSemaphoreTake( BMP280_Semaphore, ( TickType_t ) portMAX_DELAY );
    String BMP280_monitor = generateJsonString("BMP280: ", BMP280_data, TimeStamp);
    //Serial.println(BMP280_monitor);
    xSemaphoreGive( BMP280_Semaphore );

    xSemaphoreTake( BH1750_Semaphore, ( TickType_t ) portMAX_DELAY );
    String BH1750_monitor = generateJsonString("BH1750: ", String(BH1750_data), TimeStamp);
    //Serial.println(BH1750_monitor);
    xSemaphoreGive( BH1750_Semaphore );
    
    xSemaphoreTake( RTC_Semaphore, ( TickType_t ) portMAX_DELAY );
    String RTC_monitor = generateJsonString("RTC: ", TimeStamp, TimeStamp);
    //Serial.println(RTC_monitor);
    xSemaphoreGive( RTC_Semaphore );

    String monitor_message_aux = BMP280_monitor + BH1750_monitor + RTC_monitor;
    String BMP280_monitor_aux = "";
    String BH1750_monitor_aux = "";
    String RTC_monitor_aux = "";

    if(BMP280_monitor_aux != BMP280_monitor)
    {
      Serial.println(BMP280_monitor);
    }

    if(BH1750_monitor_aux != BH1750_monitor)
    {
      Serial.println(BH1750_monitor);
    }

    if(RTC_monitor_aux != RTC_monitor)
    {
      Serial.println(RTC_monitor);
    }
    Serial.println("======DELAY================================================");
    Serial.println("|BH1750: " + String(BH1750_TASK_DELAY) + "ms |BMP: " + String(BMP280_TASK_DELAY) + "ms |RTC: " + String(RTC_TASK_DELAY) + "ms |TSK: " + String(TASK_MONITOR_TASK_DELAY) + "ms");  
    Serial.println("=========================================| Uptime sys: " + String(millis() / 1000) + "s");

    if(digitalRead(LED_BUILTIN) == HIGH)
    {
      digitalWrite(LED_BUILTIN, LOW);
      LED_ON_OFF = false;
    }else if(digitalRead(LED_BUILTIN) == LOW)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      LED_ON_OFF = true;
    }

    /*
        if(monitor_message != monitor_message_aux)
    {
        monitor_message = monitor_message_aux;
        Serial.println(monitor_message);
    }
    */


    //xSemaphoreGive( I2C1_Semaphore );
  }
}

void loop() {}
