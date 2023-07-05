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



uint16_t BMP280_TASK_DELAY        = 150 + random(0, 20);
uint16_t BH1750_TASK_DELAY        = 150 + random(0, 20);
uint16_t RTC_TASK_DELAY           = 150 + random(0, 20);
uint16_t TASK_MONITOR_TASK_DELAY  = 500 + random(0, 20);


#define SLAVE_ADDRESS   0x08 // Indirizzo della Pico in modalità Slave sull'I2C0

#define RTC_REQUEST     "B" //0x42
#define BMP280_REQUEST  "C" //0x43
#define BH1750_REQUEST  "D" //0x44
bool MONITOR_LOG = true;


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

TaskHandle_t BH1750_xHandle;
TaskHandle_t BMP280_xHandle;
TaskHandle_t RTC_xHandle;
TaskHandle_t Task_monitor_xHandle;
TaskHandle_t receiveData_xHandle;

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

QueueHandle_t BH1750_queue;
QueueHandle_t BMP280_queue;
QueueHandle_t TimeStamp_queue;

String Received_Comand = "";
String Requested_Data  = "";

String BH1750_data  = "NULL";
String BMP280_data  = "NULL";
String TimeStamp    = "NULL";

unsigned long startTime;

char statsBuffer[512]; // Dimensione del buffer delle statistiche


void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  Wire.setSDA(4);
  Wire.setSCL(5);
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);

/*
}

void setup1()
{
*/
  //TaskHandle_t Task_monitor_xHandle = xTaskCreateStaticAffinitySet(TASK_MONITOR_Task, "TASK_MONITOR_Task", STACK_SIZE, NULL, configMAX_PRIORITIES - 1, xStack_TASK_MONITOR, &xTaskBuffer_TASK_MONITOR, 0);

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
  BH1750_queue      = xQueueCreate(1, sizeof(String*));
  BMP280_queue      = xQueueCreate(1, sizeof(String*));
  TimeStamp_queue   = xQueueCreate(1, sizeof(String*));

  BH1750_Semaphore = xSemaphoreCreateMutexStatic( &xBH1750_MutexBuffer );
  BMP280_Semaphore = xSemaphoreCreateMutexStatic( &xBMP280_MutexBuffer );
  RTC_Semaphore    = xSemaphoreCreateMutexStatic( &xRTC_MutexBuffer );

  I2C1_Semaphore   = xSemaphoreCreateMutexStatic( &xMutexBuffer );

  /*
    xTaskCreateStatic(BMP280_Task,       "BMP280_Task",       STACK_SIZE, NULL, configMAX_PRIORITIES - 1, xStack_BMP280,       &xTaskBuffer_BMP280);
    xTaskCreateStatic(BH1750_Task,       "BH1750_Task",       STACK_SIZE, NULL, configMAX_PRIORITIES - 1, xStack_BH1750,       &xTaskBuffer_BH1750);
    xTaskCreateStatic(RTC_Task,          "RTC_Task",          STACK_SIZE, NULL, configMAX_PRIORITIES - 1, xStack_RTC,          &xTaskBuffer_RTC);
    xTaskCreateStatic(TASK_MONITOR_Task, "TASK_MONITOR_Task", STACK_SIZE, NULL, configMAX_PRIORITIES - 2, xStack_TASK_MONITOR, &xTaskBuffer_TASK_MONITOR);
    TaskHandle_t BH1750_xHandle       = 
    TaskHandle_t BMP280_xHandle       = 
    TaskHandle_t RTC_xHandle          = 
    TaskHandle_t Task_monitor_xHandle = 
  */
      //xTaskCreate( vTaskCode,        "NAME",              STACK_SIZE, NULL, tskIDLE_PRIORITY,         &xHandle );
  xTaskCreate(BMP280_Task,       "BMP280_Task",       STACK_SIZE, NULL, configMAX_PRIORITIES - 1, &BMP280_xHandle );//xStack_BMP280,       &xTaskBuffer_BMP280);//,       1);
  xTaskCreate(BH1750_Task,       "BH1750_Task",       STACK_SIZE, NULL, configMAX_PRIORITIES - 1, &BH1750_xHandle);//xStack_BH1750,       &xTaskBuffer_BH1750);//,       1);
  xTaskCreate(RTC_Task,          "RTC_Task",          STACK_SIZE, NULL, configMAX_PRIORITIES - 1, &RTC_xHandle);//xStack_RTC,          &xTaskBuffer_RTC);   //,       1);
  //xTaskCreate(receiveData,       "receiveData_Task",  STACK_SIZE, NULL, configMAX_PRIORITIES - 1, &receiveData_xHandle);
  xTaskCreate(TASK_MONITOR_Task, "TASK_MONITOR_Task", STACK_SIZE, NULL, configMAX_PRIORITIES - 1, &Task_monitor_xHandle);//xStack_TASK_MONITOR, &xTaskBuffer_TASK_MONITOR);//, 1);

  //vTaskCoreAffinitySet(BMP280_xHandle ,      0);
  //vTaskCoreAffinitySet(BH1750_xHandle,       0);
  //vTaskCoreAffinitySet(RTC_xHandle,          0);
  //vTaskCoreAffinitySet(Task_monitor_xHandle, 1);

  //vTaskStartScheduler();
  
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
    //xSemaphoreTake( BMP280_Semaphore, ( TickType_t ) portMAX_DELAY );
    BMP280_data = BMP280_data_read();
    //Serial.println("BMP280_data from task: " + BMP280_data);
    //vTaskList(statsBuffer);
    //Serial.println(statsBuffer);
    //xSemaphoreGive( BMP280_Semaphore );

    xSemaphoreGive( I2C1_Semaphore );

    // Inserisci il valore nella coda BMP280_queue
    if(xQueueSend(BMP280_queue, &BMP280_data, 0) == pdPASS) { // La coda è piena, svuotala
      //xQueueReset(BMP280_queue);                               // Inserisci il nuovo valore nella coda
      xQueueSend(BMP280_queue, &BMP280_data, portMAX_DELAY);
      }//else xQueueSend(BMP280_queue, &BMP280_data, portMAX_DELAY);
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
    //xSemaphoreTake( BH1750_Semaphore, ( TickType_t ) portMAX_DELAY );
    BH1750_data = BH1750_data_read();
    //Serial.println("BH1750_data from task: " + BH1750_data);
    //xSemaphoreGive( BH1750_Semaphore );
    xSemaphoreGive( I2C1_Semaphore );

    // Inserisci il valore nella coda BH1750_queue
    if(xQueueSend(BH1750_queue, &BH1750_data, 0) == pdPASS) { // La coda è piena, svuotala
      //xQueueReset(BH1750_queue);                               // Inserisci il nuovo valore nella coda
      xQueueSend(BH1750_queue, &BH1750_data, portMAX_DELAY);
      }//else xQueueSend(BH1750_queue, &BH1750_data, portMAX_DELAY);
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
    //xSemaphoreTake( RTC_Semaphore, ( TickType_t ) portMAX_DELAY );
    TimeStamp = RTC_data_read();
    //Serial.println("TimeStamp from task: " + TimeStamp);
    //xSemaphoreGive( RTC_Semaphore );
    xSemaphoreGive( I2C1_Semaphore );

    // Inserisci il valore nella coda TimeStamp_queue
    if(xQueueSend(TimeStamp_queue, &TimeStamp, 0) == pdPASS) { // La coda è piena, svuotala
      //xQueueReset(TimeStamp_queue);                               // Inserisci il nuovo valore nella coda
      xQueueSend(TimeStamp_queue, &TimeStamp, portMAX_DELAY);
      }//else xQueueSend(TimeStamp_queue, &TimeStamp, portMAX_DELAY);
  }
}

//CallBack, parte in automatico quando ci sono richieste sul'I2C0

void receiveData(int byteCount){// {(void *pvParameters)

  //while(1){
    while (Wire.available()) {
      char data = Wire.read();
      // Elabora i dati ricevuti come necessario
      Serial.print("Comando ricevuto: ");
      Serial.println(data);
      Received_Comand += data;
      Serial.println(Received_Comand);
    }
  //}
}

void stringToBytes(const String& input, byte* output, size_t outputSize) {
  size_t len = input.length();
  if (len > outputSize) {
    // La stringa è più lunga dell'array di output specificato, effettua una gestione dell'errore o un'azione di fallback.
    return;
  }

  for (size_t i = 0; i < len; i++) {
    output[i] = input.charAt(i);
  }
}

void wire_write(String message)
{
    //vTaskEnterCritical();
    Wire.write(message.c_str());
    //vTaskExitCritical();
}

void sendData() {
  Serial.println("Requested_Data: " + String(Received_Comand));
  //for (size_t i = 0; i < Received_Comand.length(); i++) {
  //if(Received_Comand[i] == BMP280_REQUEST) {
  if(Received_Comand == BMP280_REQUEST) {
    Serial.println("Request = " + String(BMP280_REQUEST));

    String BMP280_data_aux = BMP280_data; //per richiamare la stringa condivisa una sola volta
    Serial.println("BMP280_data_aux: " + BMP280_data_aux);

    //xSemaphoreTake( BMP280_Semaphore, ( TickType_t ) portMAX_DELAY );
        uint8_t bmpData[BMP280_data_aux.length()];
        stringToBytes(BMP280_data_aux, bmpData, BMP280_data_aux.length());
        Serial.print("Data to send: ");
        for (size_t j = 0; j < BMP280_data_aux.length(); j++) {
          Serial.print(bmpData[j]);
          Serial.print(" ");
        }
    //xSemaphoreGive( BMP280_Semaphore );
    Serial.println();
    //Wire.write(bmpData, BMP280_data_aux.length());
    //vTaskEnterCritical();
    //Wire.write(BMP280_data_aux.c_str());
    //vTaskExitCritical();
    wire_write(BMP280_data_aux);

  }else if(Received_Comand == BH1750_REQUEST) {
    Serial.println("Request = " + String(BH1750_REQUEST));

    String BH1750_data_aux = BH1750_data; //per richiamare la stringa condivisa una sola volta
    Serial.println("BH1750_data_aux: " + BH1750_data_aux);

    //xSemaphoreTake( BH1750_Semaphore, ( TickType_t ) portMAX_DELAY );
        uint8_t bhData[BH1750_data_aux.length()];
        stringToBytes(BH1750_data_aux, bhData, BH1750_data_aux.length());
        Serial.print("Data to send: ");
        for (size_t j = 0; j < BH1750_data_aux.length(); j++) {
          Serial.print(bhData[j]);
          Serial.print(" ");
        }
    //xSemaphoreGive( BH1750_Semaphore );
    Serial.println();

    //Wire.write(bhData, BH1750_data_aux.length());
    wire_write(BH1750_data_aux);
    //Wire.write(BH1750_data_aux.c_str());

  }else if(Received_Comand == String(RTC_REQUEST)) {
    //Serial.println("Request = " + RTC_REQUEST);

    String TimeStamp_aux = TimeStamp; //per richiamare la stringa condivisa una sola volta
    Serial.println("TimeStamp_aux: " + TimeStamp_aux);

    //xSemaphoreTake( RTC_Semaphore, ( TickType_t ) portMAX_DELAY );
        uint8_t timeData[TimeStamp_aux.length()];
        stringToBytes(TimeStamp_aux, timeData, TimeStamp_aux.length());
        Serial.print("Data to send: ");
        for (size_t j = 0; j < TimeStamp_aux.length(); j++) {
          Serial.print(timeData[j]);
          Serial.print(" ");
        }
    //xSemaphoreGive( RTC_Semaphore );
    Serial.println();
    //Wire.write(TimeStamp_aux.c_str());
    wire_write(TimeStamp_aux);
  } else {
    // Richiesta non valida, invia messaggio di errore
    Serial.println("INVALID REQUEST!");
    String ERROR = "error";
    //Wire.write(ERROR.c_str());
    wire_write(ERROR);
  }

  Received_Comand = "";
}



/*
void sendData() {
  Serial.println("e");
  Serial.println(Requested_Data);
  Wire.write("a");
  
  for (size_t i = 0; i < Requested_Data.length(); i++) {
    if (Requested_Data[i] == BMP280_REQUEST) {
      char bmpData[BMP280_data.length() + 1];
      BMP280_data.toCharArray(bmpData, BMP280_data.length() + 1);
      Wire.write(bmpData);
    }
    if (Requested_Data[i] == BH1750_REQUEST) {
      char bhData[BH1750_data.length() + 1];
      BH1750_data.toCharArray(bhData, BH1750_data.length() + 1);
      Wire.write(bhData);
    }
    if (Requested_Data[i] == RTC_REQUEST) {
      char timeData[TimeStamp.length() + 1];
      TimeStamp.toCharArray(timeData, TimeStamp.length() + 1);
      Wire.write(timeData);
    }
  }
  
  Requested_Data = "";
  Serial.println("f");
}
*/

/*
void sendData_task(void* pvParameters) {
  //while (1) {
    // Resto del codice della funzione sendData()
    // ...
    for (size_t i = 0; i < Requested_Data.length(); i++) {
      if(Requested_Data[i] == BMP280_REQUEST)
        Wire.write(BMP280_data, BMP280_data.length());
      if(Requested_Data[i] == BH1750_REQUEST)
        Wire.write(BH1750_data, BH1750_data.length());
      if(Requested_Data[i] == RTC_REQUEST)
        Wire.write(TimeStamp, TimeStamp.length());
    Requested_Data = "";
    //}
    //vTaskDelay(pdMS_TO_TICKS(10));  // Aggiungi una piccola pausa per evitare un loop continuo
  }
}
*/

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

void LED_Control(bool *LED_ON_OFF)
{
    if(digitalRead(LED_BUILTIN) == HIGH)
  {
    digitalWrite(LED_BUILTIN, LOW);
    *LED_ON_OFF = false;
  }else if(digitalRead(LED_BUILTIN) == LOW)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    *LED_ON_OFF = true;
  }
}

void TASK_MONITOR_Task(void *pvParameters)
{
  (void) pvParameters;
  /*
      SETUP HERE
  */
  String monitor_message = "";
  String BMP280_monitor  = "";
  String BH1750_monitor  = "";
  String RTC_monitor     = "";

  Serial.println(F("TASK MONITOR started..."));

  bool LED_ON_OFF = false;

  while (1)
  {
    LED_Control(&LED_ON_OFF);
    
    BMP280_TASK_DELAY        = 150 + random(0, 20);
    BH1750_TASK_DELAY        = 150 + random(0, 20);
    RTC_TASK_DELAY           = 150 + random(0, 20);
    TASK_MONITOR_TASK_DELAY  = 500 + random(0, 20);
    
    vTaskDelay(TASK_MONITOR_TASK_DELAY / portTICK_PERIOD_MS);
    //xSemaphoreTake( I2C1_Semaphore, ( TickType_t ) portMAX_DELAY );
    /*
        LOOP HERE
    */

    // Sospendi temporaneamente i task BMP280_Task, BH1750_Task, RTC_Task
//    vTaskSuspend(BMP280_xHandle);
//    vTaskSuspend(BH1750_xHandle);
//    vTaskSuspend(RTC_xHandle);
    
    if (xQueueReceive(BMP280_queue, &BMP280_data, 0) == pdTRUE) {
      BMP280_monitor = generateJsonString("BMP280: ", BMP280_data, TimeStamp);
    }

    if (xQueueReceive(BH1750_queue, &BH1750_data, 0) == pdTRUE) {
      BH1750_monitor = generateJsonString("BH1750: ", BH1750_data, TimeStamp);
    }

    if (xQueueReceive(TimeStamp_queue, &TimeStamp, 0) == pdTRUE) {
      RTC_monitor = generateJsonString("RTC: ", TimeStamp, TimeStamp);
    }

//    vTaskResume(BMP280_xHandle);
//    vTaskResume(BH1750_xHandle);
//    vTaskResume(RTC_xHandle);
 
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

    //vTaskGetRunTimeStats(statsBuffer);ù
    //vTaskList(statsBuffer);

    
    if(MONITOR_LOG){
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
    }

    //Serial.println(statsBuffer);
    //Serial.println("=========================================");


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
