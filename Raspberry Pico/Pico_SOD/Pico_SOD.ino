//Arduino libraries
#include <ArduinoJson.h>
#include <cstring>

#include <Wire.h> 
//#include <HardwareSerial.h>
//#include <SoftwareSerial.h>

#include "sensors\BMP280_sensor.hpp"
#include "sensors\BH1750_sensor.hpp"
#include "sensors\RTC_sensor.hpp"
#include "variables_definition.hpp"

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

bool COMMUNICATION_MODE = Serial1_MODE;
bool MONITOR_LOG = false;

//SoftwareSerial Serial1(Serial1_TX_PIN, Serial1_RX_PIN);

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  Wire.setSDA(I2C0_SDA_PIN);
  Wire.setSCL(I2C0_SCL_PIN);
  //Wire.setClock(I2C0_CLOCK);
  //Wire.setTimeout(I2C0_TIMEOUT);

  //if(COMMUNICATION_MODE == I2C_MODE){
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveData);
  //Wire.onReceive(receiveEvent);
  Wire.onRequest(sendData_I2C);
  //}

  // Create a SoftwareSerial object for Serial1 communication
  //Serial1.setPinout(Serial1_TX_PIN, Serial1_RX_PIN);
  //Serial1.begin(115200);
  //if(COMMUNICATION_MODE == Serial1_MODE){
  //Serial1.setRX(Serial1_RX_PIN);
  //Serial1.setTX(Serial1_TX_PIN);
  //  //Serial1.setFIFOSize(128);
  //  Serial1.setPinout(Serial1_TX_PIN, Serial1_RX_PIN);
  //  //Serial1.begin(115200, SERIAL_8N1, 12, 13);
  //Serial1.begin(115200);
  //  //Serial1.begin(9600);
  //}
  

  //Definizione PIN ed inizializzazione I2C1 in Master
  Wire1.setSDA(I2C1_SDA_PIN);
  Wire1.setSCL(I2C1_SCL_PIN);
  Wire1.begin();

  delay(2000);
  Serial.println(F("Avvio...."));
  delay(500);

  startTime = millis();  // Salva il tempo di avvio iniziale

  //BMP_TEMP_QUEUE    = xQueueCreate(1000, sizeof(String*));
  //BMP_PRESS_QUEUE   = xQueueCreate(1000, sizeof(String*));
  //BMP_ALT_QUEUE     = xQueueCreate(1000, sizeof(String*));
  //BH1750_QUEUE      = xQueueCreate(1000, sizeof(String*));
  //TIMESTAMP_QUEUE   = xQueueCreate(1000, sizeof(String*));
  
  //BH1750_Semaphore = xSemaphoreCreateMutexStatic( &xBH1750_MutexBuffer );
  //BMP280_Semaphore = xSemaphoreCreateMutexStatic( &xBMP280_MutexBuffer );
  //RTC_Semaphore    = xSemaphoreCreateMutexStatic( &xRTC_MutexBuffer );

  I2C1_Semaphore   = xSemaphoreCreateMutexStatic( &xMutexBuffer );

  xTaskCreate(BMP280_Task,           "BMP280_Task",                STACK_SIZE, NULL, configMAX_PRIORITIES - 3, &BMP280_xHandle );
  xTaskCreate(BH1750_Task,           "BH1750_Task",                STACK_SIZE, NULL, configMAX_PRIORITIES - 2, &BH1750_xHandle);
  xTaskCreate(RTC_Task,              "RTC_Task",                   STACK_SIZE, NULL, configMAX_PRIORITIES - 4, &RTC_xHandle);
  xTaskCreate(TASK_MONITOR_Task,     "TASK_MONITOR_Task",          STACK_SIZE, NULL, configMAX_PRIORITIES - 1, &Task_monitor_xHandle);

  //vTaskStartScheduler();
  //if(COMMUNICATION_MODE == Serial1_MODE){
  //    xTaskCreate(Serial1_Communication, "Serial1_Communication_Task", STACK_SIZE, NULL, configMAX_PRIORITIES - 1, &Serial1_Communication_xHandle);
  //}
}

void Serial1_Communication(void *pvParameters) {
  while (Serial1.available()) {
    char data = Serial1.read();
    Serial.print("Serial1 MSG RECEIVED: ");
    Serial.println(data);
  }
  
}

void BMP280_Task(void *pvParameters)
{
  (void) pvParameters;

  Serial.println(F("Setup BMP280...."));
  BMP280_setup();
  delay(500);

  while (1)
  {
    vTaskDelay(BMP280_TASK_DELAY / portTICK_PERIOD_MS);

    if(xSemaphoreTake( I2C1_Semaphore, ( TickType_t ) portMAX_DELAY ) == pdTRUE)
    {
      BMP_TEMP_VALUE = BMP280_data_temp();
      BMP_PRESS_VALUE= BMP280_data_press();
      BMP_ALT_VALUE  = BMP280_data_alt();

      //Serial.println(BMP280_data[0]);
      xSemaphoreGive( I2C1_Semaphore );
      
      // Inserisci il valore nella coda BMP280_queue
      /*
      if(xQueueSend(BMP_TEMP_QUEUE, &BMP_TEMP_VALUE, 0) == pdPASS) { 
          xQueueReset(BMP_TEMP_QUEUE);                               
          xQueueSend(BMP_TEMP_QUEUE, &BMP_TEMP_VALUE, portMAX_DELAY);
        }
    
      if(xQueueSend(BMP_PRESS_QUEUE, &BMP_PRESS_VALUE, 0) == pdPASS) {
          xQueueReset(BMP_PRESS_QUEUE);                               
          xQueueSend(BMP_PRESS_QUEUE, &BMP_PRESS_VALUE, portMAX_DELAY);
        }

      if(xQueueSend(BMP_ALT_QUEUE, &BMP_ALT_VALUE, 0) == pdPASS) {
          xQueueReset(BMP_ALT_QUEUE);                               
          xQueueSend(BMP_ALT_QUEUE, &BMP_ALT_VALUE, portMAX_DELAY);
        }
      */

    }else {
      return;
    }
  }
}

void BH1750_Task(void *pvParameters)
{
  (void) pvParameters;

  Serial.println(F("Setup BH1750...."));
  BH1750_setup();
  delay(500);

  while (1)
  {
    vTaskDelay(BH1750_TASK_DELAY / portTICK_PERIOD_MS);
    if(xSemaphoreTake( I2C1_Semaphore, ( TickType_t ) portMAX_DELAY ) == pdTRUE)
    {
      BH1750_LUX_VALUE = BH1750_data_read();
      xSemaphoreGive( I2C1_Semaphore );

      /*
      //Inserisci il valore nella coda BH1750_queue
      if(xQueueSend(BH1750_QUEUE, &BH1750_LUX_VALUE, 0) == pdPASS) { // La coda è piena, svuotala
        xQueueReset(BH1750_QUEUE);                               // Inserisci il nuovo valore nella coda
        xQueueSend(BH1750_QUEUE, &BH1750_LUX_VALUE, portMAX_DELAY);
        }
      */
    }else  {
      return;
    }
  }
}

void RTC_Task(void *pvParameters)
{
  (void) pvParameters;

  Serial.println(F("Setup RTC...."));
  RTC_setup();
  delay(500);

  while (1)
  {
    vTaskDelay(RTC_TASK_DELAY / portTICK_PERIOD_MS);
    if(xSemaphoreTake( I2C1_Semaphore, ( TickType_t ) portMAX_DELAY ) == pdTRUE)
    {
      RTC_DATA_VALUE = RTC_data_read();
      xSemaphoreGive( I2C1_Semaphore );
      //Serial.println("RTC_DATA_VALUE: " + String(RTC_DATA_VALUE));
    }else {
      return;
    }

    /*
    // Inserisci il valore nella coda TimeStamp_queue
    if(xQueueSend(TIMESTAMP_QUEUE, &RTC_DATA_VALUE, 10) != pdPASS) {
        //Serial.println("CODA PIENA");
        xQueueReset(TIMESTAMP_QUEUE);
        //Serial.println("CODA RESETTATA");
      xQueueSend(TIMESTAMP_QUEUE, &RTC_DATA_VALUE, portMAX_DELAY);
        //Serial.println("CODA CON NUOVO DATO");
      }else Serial.println("CODA NON AGGIORNATA");
    */
  }
}

//CallBack, parte in automatico quando ci sono richieste sul'I2C0

void splitDateString(const String& dateString, uint16_t& year, uint8_t& month, uint8_t& day, uint8_t& hour, uint8_t& minute, uint8_t& second) {
  year    =   (dateString.substring(0, 4)).toInt();
  month   =   (dateString.substring(5, 7)).toInt();
  day     =   (dateString.substring(8, 10)).toInt();
  hour    =   (dateString.substring(11, 13)).toInt();
  second  =   (dateString.substring(17, 19)).toInt();
  minute  =   (dateString.substring(14, 16)).toInt();
}

/*
void receiveEvent(int howMany)
{
  while(1 < Wire.available()) // loop through all but the last
  {
    char c = Wire.read(); // receive byte as a character
    Serial.print(c);         // print the character
  }
  int x = Wire.read();    // receive byte as an integer
  Serial.println(x);         // print the integer
}
*/
void receiveData_new(int byteCount) {
  if (0 < Wire.available()) {
    char receivedData = Wire.read();  // Leggi il dato ricevuto
    // Fai qualcosa con il dato ricevuto
    // In questo esempio, stampiamo il dato sulla porta seriale
    Serial.print("Dato ricevuto: ");
    Serial.println(receivedData);
  }
}

void string_to_byte_array(String Sync_Time_String, byte byteArray[])
{
  char* token = strtok(const_cast<char*>(Sync_Time_String.c_str()), ",");
  int index = 0;

  while (token != NULL && index < 4) {
    byteArray[index++] = atoi(token);
    token = strtok(NULL, ",");
  }
  Serial.print("byteArray: ");
  Serial.print(String(byteArray[0]) + ", ");
  Serial.print(String(byteArray[1]) + ", ");
  Serial.print(String(byteArray[2]) + ", ");
  Serial.println(String(byteArray[3])); 
}

void receiveData(int byteCount) {
  bool SYNC_TS = false;
  uint8_t _count = 0;
  Sync_Time_String = "";
  byte byteArray[4];
  
  while (0 < Wire.available()) {
    _count++;
    char data = Wire.read();
    Serial.println("data: " + String(data));
    Serial.println("count: " + String(_count));

    if((data == BMP_TEMP_REG || data == BMP_PRESS_REG || data == BMP_ALT_REG || data == BH1750_LUX_REG || data == RTC_REG) && !SYNC_TS){
      Received_Comand = data;
      data_string = "";
    }else if(data == SYNC_TIME && !SYNC_TS){
      SYNC_TS = true;
      Received_Comand = data;
    }else if(SYNC_TS){
      Sync_Time_String += byte(data);
      if(_count != 5){
        Sync_Time_String += ",";
      }
      Serial.print("Sync_Time_String: ");
      Serial.println(Sync_Time_String); 
    }
  }
  if(SYNC_TS){
    string_to_byte_array(Sync_Time_String, byteArray);
  }
  
  SYNC_TS = false;
}

void sendData_I2C() {
  Serial.println("Requested_Data: " + String(Received_Comand));
  
  switch (Received_Comand) {
    case BMP_TEMP_REG: //A , 0x41
      Serial.println("In CASE A");
      Wire.write((byte)BMP_TEMP_VALUE);
    break;
      
    case BMP_PRESS_REG: //B , 0x42
      Serial.println("In CASE B");
      Wire.write((byte)BMP_PRESS_VALUE);
    break;
      
    case BMP_ALT_REG: //C , 0x43
      Serial.println("In CASE C");
      Wire.write((byte)BMP_ALT_VALUE);
    break;
      
    case BH1750_LUX_REG: //D , 0x44
      Serial.println("In CASE D");
      Wire.write((byte)BH1750_LUX_VALUE);
    break;

    case RTC_REG: //E , 0x45

      byte byteArray[4];
      byteArray[0] = (RTC_DATA_VALUE >> 24) & 0xFF; 
      byteArray[1] = (RTC_DATA_VALUE >> 16) & 0xFF; 
      byteArray[2] = (RTC_DATA_VALUE >> 8) & 0xFF; 
      byteArray[3] = RTC_DATA_VALUE & 0xFF; 

      Serial.println("In CASE E");
      Wire.write(byteArray, sizeof(byteArray));
      break;

    case SYNC_TIME: //F, 0x46
      
      break;
    default:
      // Richiesta non valida, invia messaggio di errore
      Serial.println("INVALID REQUEST!");
      String errorMessage = "Error occurred";
      data_string = "";
      Wire.write(errorMessage.c_str(), errorMessage.length());
      break;
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
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - startTime;
  unsigned int seconds = elapsedTime / 1000;
  Serial.println(seconds);
  delay(1000);
}

void LED_Control(bool* LED_ON_OFF) {
  if (digitalRead(LED_BUILTIN) == HIGH) {
    digitalWrite(LED_BUILTIN, LOW);
    *LED_ON_OFF = false;
  } else if (digitalRead(LED_BUILTIN) == LOW) {
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

  Serial.println(F("TASK MONITOR started..."));

  bool LED_ON_OFF = false;

  while (1)
  {
    LED_Control(&LED_ON_OFF);
    
    /*
        LOOP HERE
    */
    vTaskDelay(TASK_MONITOR_TASK_DELAY / portTICK_PERIOD_MS);

    /*
    if (xQueuePeek(BMP_TEMP_QUEUE, &BMP_TEMP_VALUE, 100) == pdTRUE) {
      TEMP_BMP280_MONITOR = BMP_TEMP_VALUE;//generateJsonString("BMP280: ", String(BMP_TEMP_VALUE), String(RTC_DATA_VALUE));
    }

    if (xQueuePeek(BMP_PRESS_QUEUE, &BMP_PRESS_VALUE, 100) == pdTRUE) {
      PRESS_BMP280_MONITOR = BMP_PRESS_VALUE; //generateJsonString("BMP280: ", String(BMP_PRESS_VALUE), String(RTC_DATA_VALUE));
    }

    if (xQueuePeek(BMP_ALT_QUEUE, &BMP_ALT_VALUE, 100) == pdTRUE) {
      ALT_BMP280_MONITOR = BMP_ALT_VALUE; //generateJsonString("BMP280: ", String(BMP_ALT_VALUE), String(RTC_DATA_VALUE));
    }

    if (xQueuePeek(BH1750_QUEUE, &BH1750_LUX_VALUE, 100) == pdTRUE) {
      BH1750_MONITOR = BH1750_LUX_VALUE; //generateJsonString("BH1750: ", String(BH1750_LUX_VALUE), String(RTC_DATA_VALUE));
    }

    if (xQueuePeek(TIMESTAMP_QUEUE, &RTC_DATA_VALUE, 10) == pdTRUE) {
      RTC_MONITOR = RTC_DATA_VALUE; //generateJsonString("RTC: ", String(RTC_DATA_VALUE), String(RTC_DATA_VALUE));
    }
    */

    float TEMP_BMP280_MONITOR_AUX  = 0.0;
    float PRESS_BMP280_MONITOR_AUX = 0.0;
    float ALT_BMP280_MONITOR_AUX   = 0.0;
    float BH1750_MONITOR_AUX       = 0.0;
    uint32_t RTC_MONITOR_AUX       = 0;

    if(MONITOR_LOG){
        if(TEMP_BMP280_MONITOR_AUX != BMP_TEMP_VALUE)
            {
                Serial.print("|TEMP: ");
                Serial.println(BMP_TEMP_VALUE);
                TEMP_BMP280_MONITOR_AUX = BMP_TEMP_VALUE;
            }

        if(PRESS_BMP280_MONITOR_AUX != BMP_PRESS_VALUE)
            {
                Serial.print("|PRESS: ");
                Serial.println(BMP_PRESS_VALUE);
                PRESS_BMP280_MONITOR_AUX = BMP_PRESS_VALUE;
            }
        if(ALT_BMP280_MONITOR_AUX != BMP_ALT_VALUE)
            {
                Serial.print("|ALT: ");
                Serial.println(BMP_ALT_VALUE);
                ALT_BMP280_MONITOR_AUX = BMP_ALT_VALUE;
            }

        if(BH1750_MONITOR_AUX != BH1750_LUX_VALUE)
            {
                Serial.print("|LUX: ");
                Serial.println(BH1750_LUX_VALUE);
                BH1750_MONITOR_AUX = BH1750_LUX_VALUE;
            }

        if(RTC_MONITOR_AUX != RTC_DATA_VALUE)
            {
                Serial.print("|RTC: ");
                Serial.println(RTC_DATA_VALUE);
                RTC_MONITOR_AUX = RTC_DATA_VALUE;
            }

        Serial.println("======DELAY================================================");
        Serial.println("|BH1750: " + String(BH1750_TASK_DELAY) + "ms |BMP: " + String(BMP280_TASK_DELAY) + "ms |RTC: " + String(RTC_TASK_DELAY) + "ms |TSK: " + String(TASK_MONITOR_TASK_DELAY) + "ms");  
        Serial.println("=========================================| Uptime sys: " + String(millis() / 1000) + "s");
        
        //printQueue();
    }
  }
}

void loop() {}
