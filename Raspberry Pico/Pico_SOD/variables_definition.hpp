#ifndef VARIABLES_DEFINITION_CLASS_H
#define VARIABLES_DEFINITION_CLASS_H

//Librerie Arduino
#include <Arduino.h>

uint16_t BMP280_TASK_DELAY        = 150 + random(0, 20);
uint16_t BH1750_TASK_DELAY        = 150 + random(0, 20);
uint16_t RTC_TASK_DELAY           = 150 + random(0, 20);
uint16_t TASK_MONITOR_TASK_DELAY  = 500 + random(0, 20);


#define SLAVE_ADDRESS   0x08 // Indirizzo della Pico in modalità Slave sull'I2C0

// Registri dei sensori
#define BMP_TEMP_REG    0x41
#define BMP_PRESS_REG   0x42
#define BMP_ALT_REG     0x43
#define BH1750_LUX_REG  0x44
#define RTC_REG         0x45
#define SYNC_TIME       0x46

#define Serial1_TX_PIN 12
#define Serial1_RX_PIN 13
#define I2C0_SDA_PIN 4  
#define I2C0_SCL_PIN 5
#define I2C1_SDA_PIN 2
#define I2C1_SCL_PIN 3
#define I2C0_CLOCK   100000
#define I2C0_TIMEOUT 1000

#define ERROR_CODE 0xab

bool Serial1_MODE = true;
bool I2C_MODE  = true;

#define STACK_SIZE 10000

TaskHandle_t BH1750_xHandle;
TaskHandle_t BMP280_xHandle;
TaskHandle_t RTC_xHandle;
TaskHandle_t Task_monitor_xHandle;
TaskHandle_t receiveData_xHandle;
TaskHandle_t Serial1_Communication_xHandle;

SemaphoreHandle_t I2C1_Semaphore  ;

StaticSemaphore_t xMutexBuffer;

StaticSemaphore_t xI2C1_MutexBuffer;

QueueHandle_t BMP_TEMP_QUEUE;
QueueHandle_t BMP_PRESS_QUEUE;
QueueHandle_t BMP_ALT_QUEUE;
QueueHandle_t BH1750_QUEUE;
QueueHandle_t TIMESTAMP_QUEUE;

QueueHandle_t TEST_QUEUE;

bool LED_ON_OFF = false;

char Received_Command;

float    BMP_TEMP_VALUE   = 0; 
float    BMP_PRESS_VALUE  = 0; 
float    BMP_ALT_VALUE    = 0; 
float    BH1750_LUX_VALUE = 0; 
int      RTC_DATA_VALUE   = 0; 
String   Sync_Time_String   = "";

unsigned long startTime;
String data_string = "";

char statsBuffer[512]; // Dimensione del buffer delle statistiche

#endif
