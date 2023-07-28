// Librerie Arduino
#include <cstring>
#include <Wire.h>
#include "sensors\BMP280_sensor.hpp"
#include "sensors\BH1750_sensor.hpp"
#include "sensors\RTC_sensor.hpp"
#include "variables_definition.hpp"
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

// Variabili globali
bool MONITOR_LOG = false;                  // Flag per il log del monitoraggio
bool SEMAPHORE_I2C1_ENABLED = true;        // Flag per l'abilitazione del semaforo I2C1

// Funzione di setup, eseguita una volta all'avvio
void setup() {
  // Inizializza la comunicazione seriale
  Serial.begin(9600);

  // Imposta il pin del LED integrato come output
  pinMode(LED_BUILTIN, OUTPUT);

  // Configura i pin I2C0
  Wire.setSDA(I2C0_SDA_PIN);
  Wire.setSCL(I2C0_SCL_PIN);

  // Inizia la comunicazione I2C come slave con l'indirizzo specificato
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveData);    // Registra la funzione receiveData come callback per la ricezione dei dati su I2C
  Wire.onRequest(sendData_I2C);   // Registra la funzione sendData_I2C come callback per l'invio dei dati su I2C

  // Definisce e inizializza i pin I2C1 come master
  Wire1.setSDA(I2C1_SDA_PIN);
  Wire1.setSCL(I2C1_SCL_PIN);
  Wire1.begin();

  // Aggiungi un ritardo per l'inizializzazione
  delay(2000);
  Serial.println(F("Avvio...."));
  delay(500);

  // Salva il tempo di avvio iniziale
  startTime = millis();

  // Crea e inizializza un mutex per il semaforo di accesso I2C1
  I2C1_Semaphore = xSemaphoreCreateMutexStatic(&xMutexBuffer);

  // Crea le attività per i diversi sensori e il monitoraggio
  xTaskCreate(BMP280_Task, "BMP280_Task", STACK_SIZE, NULL, configMAX_PRIORITIES - 1, &BMP280_xHandle);
  xTaskCreate(BH1750_Task, "BH1750_Task", STACK_SIZE, NULL, configMAX_PRIORITIES - 2, &BH1750_xHandle);
  xTaskCreate(RTC_Task, "RTC_Task", STACK_SIZE, NULL, configMAX_PRIORITIES - 4, &RTC_xHandle);
  xTaskCreate(TASK_MONITOR_Task, "TASK_MONITOR_Task", STACK_SIZE, NULL, configMAX_PRIORITIES - 1, &Task_monitor_xHandle);
}

// Attività per la lettura dei dati dal sensore BMP280
void BMP280_Task(void *pvParameters) {
  (void) pvParameters;

  Serial.println(F("Configurazione BMP280...."));
  BMP280_setup(); // Setup del sensore BMP280
  delay(500);

  while (1) {
    vTaskDelay(BMP280_TASK_DELAY / portTICK_PERIOD_MS);

    // Tentativo di acquisire il semaforo, se questa modalità è abilitata
    if ((xSemaphoreTake(I2C1_Semaphore, (TickType_t) portMAX_DELAY) == pdTRUE) || SEMAPHORE_I2C1_ENABLED == true) {
      BMP_TEMP_VALUE = BMP280_data_temp();
      BMP_PRESS_VALUE = BMP280_data_press();
      BMP_ALT_VALUE = BMP280_data_alt();

      xSemaphoreGive(I2C1_Semaphore); // Rilascio del semaforo
    } else if (SEMAPHORE_I2C1_ENABLED) {// Se l'acquisizione del semaforo fallisce il il task non esegue l'interrogazione del sensore,
      return;                           // lasciando nelle variabili globali e dati della precedente acquisizione
    } else {  
      BMP_TEMP_VALUE = BMP280_data_temp(); // Se il semaforo è disabilitato, il task interroga direttamente il sensore
      BMP_PRESS_VALUE = BMP280_data_press();
      BMP_ALT_VALUE = BMP280_data_alt();
    }
  }
}

// Attività per la lettura dei dati dal sensore BH1750
void BH1750_Task(void *pvParameters) {
  (void) pvParameters;

  Serial.println(F("Configurazione BH1750...."));
  BH1750_setup(); // Setup del sensore BH1750
  delay(500);

  while (1) {
    vTaskDelay(BH1750_TASK_DELAY / portTICK_PERIOD_MS);

    // Tentativo di acquisire il semaforo, se questa modalità è abilitata
    if ((xSemaphoreTake(I2C1_Semaphore, (TickType_t) portMAX_DELAY) == pdTRUE) || SEMAPHORE_I2C1_ENABLED) {
      BH1750_LUX_VALUE = BH1750_data_read();
      xSemaphoreGive(I2C1_Semaphore);  // Rilascio del semaforo
    } else if (SEMAPHORE_I2C1_ENABLED) { // Se l'acquisizione del semaforo fallisce il il task non esegue l'interrogazione del sensore,
      return;                            // lasciando nelle variabili globali e dati della precedente acquisizione
    } else {
      BH1750_LUX_VALUE = BH1750_data_read(); // Se il semaforo è disabilitato, il task interroga direttamente il sensore
    }
  }
}

// Attività per la lettura dei dati dal sensore RTC
void RTC_Task(void *pvParameters) {
  (void) pvParameters;

  Serial.println(F("Configurazione RTC...."));
  RTC_setup(); // Setup dell'RTC
  delay(500);

  while (1) {
    vTaskDelay(RTC_TASK_DELAY / portTICK_PERIOD_MS);

    // Tentativo di acquisire il semaforo, se questa modalità è abilitata
    if ((xSemaphoreTake(I2C1_Semaphore, (TickType_t) portMAX_DELAY) == pdTRUE) || SEMAPHORE_I2C1_ENABLED) {
      RTC_DATA_VALUE = RTC_data_read();
      xSemaphoreGive(I2C1_Semaphore); // Rilascio del semaforo
    } else if (SEMAPHORE_I2C1_ENABLED) { // Se l'acquisizione del semaforo fallisce il il task non esegue l'interrogazione del sensore,
      return;                            // lasciando nelle variabili globali e dati della precedente acquisizione
    } else {
      RTC_DATA_VALUE = RTC_data_read(); // Se il semaforo è disabilitato, il task interroga direttamente il sensore
    }
  }
}

// Funzione per convertire una stringa in un array di byte e un timestamp
unsigned long string_to_byte_array(String Sync_Time_String, byte byteArray[]) {
  char *token = strtok(const_cast<char *>(Sync_Time_String.c_str()), ",");
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

  // Combina gli elementi dell'array di byte in un intero a 32 bit
  unsigned long combinedInt = (byteArray[0] << 24) | (byteArray[1] << 16) | (byteArray[2] << 8) | byteArray[3];
  // Utilizza il valore combinato come timestamp
  unsigned long timestamp = combinedInt;
  Serial.println(timestamp);

  return timestamp;
}

// Funzione per gestire i dati ricevuti su I2C tramite OnReceive()
void receiveData(int byteCount) {
  bool SYNC_TS = false; // La funzioni parte con il flag disabilitato
  uint8_t _count = 0;
  Sync_Time_String = "";
  byte byteArray[4];

  while (0 < Wire.available()) {
    _count++;
    char data = Wire.read(); // Acquisisce i byte finchè sono disponibili
    
    /*

    Se il byte corrisponde ad uno tra i registri (data === BMP_TEMP_REG ad esempio), 
    ed il flag è disabilitato (cosa sempre vera nel primo byte), allora il reg viene copiato in Received_Command.
    Se invece il reg corrisponde al SYNC_TIME ed il flag è disabilitato
    il flag viene settato a "true" e Received_Command diventa il valore di SYNC_TIME.
    Nelle iterazioni successive il flag sarà abilitato (nel caso di SYNC_TIME) e quindi
    si accede all'else if finale dove viene ricostruita la sequenza di byte che
    rappresenta il timestamp e se il count è diverso da 5 viene aggiunta una virgola per 
    separare i singoli byte che compongono il timestamp.

    */ 


    if ((data == BMP_TEMP_REG || data == BMP_PRESS_REG || data == BMP_ALT_REG || data == BH1750_LUX_REG || data == RTC_REG) && !SYNC_TS) {
      Received_Command = data; 
      data_string = "";        
    } else if (data == SYNC_TIME && !SYNC_TS) { 
      SYNC_TS = true;                           
      Received_Command = data;
    } else if (SYNC_TS) {              
      Sync_Time_String += byte(data);  
      if (_count != 5) {
        Sync_Time_String += ",";
      }
    }
  }
  /*
    Una volta conclusa la ricezione del o dei byte, se il flag è abilitato si effettua la conversione da stringa
    a byteArray e si crea un oggetto datetime contenente il timestamp che andrà successivamente a settare l'RTC.
    Una volta conclusa l'operazione, il flag torna a "false" per permettere la ricezione di un'altra richiesta.
  */
  if (SYNC_TS) {
    unsigned long ts = string_to_byte_array(Sync_Time_String, byteArray);
    DateTime dateTime = DateTime(ts); // Imposta il timestamp desiderato
    rtc.adjust(dateTime);
  }

  SYNC_TS = false;
}


/*
  Funzione per inviare dati su I2C come richiesto

  sendData_I2C viene richiamata dalla funzione OnRequest() e si occupa di andare a leggere il byte ricevuto e salvato nella
  variabile globale Received_Command.
  Successivamente in base al registro ricevuto, verrà effettuato il "write" del dato corrispondente al sensore richiesto
  sull'I2C0.

*/
void sendData_I2C() {
  Serial.print(F("Richiesta dati: "));
  Serial.print(String(Received_Command));

  switch (Received_Command) {
    case BMP_TEMP_REG: //A , 0x41
      Wire.write((byte) BMP_TEMP_VALUE);
      break;

    case BMP_PRESS_REG: //B , 0x42
      Wire.write((byte) BMP_PRESS_VALUE);
      break;

    case BMP_ALT_REG: //C , 0x43
      Wire.write((byte) BMP_ALT_VALUE);
      break;

    case BH1750_LUX_REG: //D , 0x44
      Wire.write((byte) BH1750_LUX_VALUE);
      break;

    case RTC_REG: //E , 0x45
      byte byteArray[4];
      byteArray[0] = (RTC_DATA_VALUE >> 24) & 0xFF;
      byteArray[1] = (RTC_DATA_VALUE >> 16) & 0xFF;
      byteArray[2] = (RTC_DATA_VALUE >> 8) & 0xFF;
      byteArray[3] = RTC_DATA_VALUE & 0xFF;
      Wire.write(byteArray, sizeof(byteArray));
      break;

    case SYNC_TIME: //F, 0x46
      // Azioni per gestire la sincronizzazione del tempo
      break;

    default:
      // Richiesta non valida, invia messaggio di errore
      Serial.println(F("RICHIESTA NON VALIDA!"));
      String errorMessage = "Errore";
      data_string = "";
      Wire.write(errorMessage.c_str(), errorMessage.length());
      break;
  }
}

// Funzione per il controllo del LED integrato
void LED_Control(bool *LED_ON_OFF) {
  if (digitalRead(LED_BUILTIN) == HIGH) {
    digitalWrite(LED_BUILTIN, LOW);
    *LED_ON_OFF = false;
  } else if (digitalRead(LED_BUILTIN) == LOW) {
    digitalWrite(LED_BUILTIN, HIGH);
    *LED_ON_OFF = true;
  }
}

// Attività di monitoraggio

/*

  Questo task si occupa principalmente far lampeggiare il LED di bordo della Pico ad ogni iterazione
  per permettere di capire, anche solo visivamente, se la board sta funzionando; opzionalmente, se il flag "MONITOR_LOG"
  è abilitato, stampa su seriale tutti i dati dalle variabili globali dei singoli sensori per permette 
  una più semplice diagnostica degli stessi.

*/
void TASK_MONITOR_Task(void *pvParameters) {
  (void) pvParameters;

  Serial.println(F("Monitoraggio delle attività avviato..."));

  bool LED_ON_OFF = false;

  while (1) {
    LED_Control(&LED_ON_OFF);

    vTaskDelay(TASK_MONITOR_TASK_DELAY / portTICK_PERIOD_MS);

    float TEMP_BMP280_MONITOR_AUX = 0.0;
    float PRESS_BMP280_MONITOR_AUX = 0.0;
    float ALT_BMP280_MONITOR_AUX = 0.0;
    float BH1750_MONITOR_AUX = 0.0;
    uint32_t RTC_MONITOR_AUX = 0;

    if (MONITOR_LOG) {
      if (TEMP_BMP280_MONITOR_AUX != BMP_TEMP_VALUE) {
        Serial.print("|TEMP: ");
        Serial.println(BMP_TEMP_VALUE);
        TEMP_BMP280_MONITOR_AUX = BMP_TEMP_VALUE;
      }
      if (PRESS_BMP280_MONITOR_AUX != BMP_PRESS_VALUE) {
        Serial.print("|PRESS: ");
        Serial.println(BMP_PRESS_VALUE);
        PRESS_BMP280_MONITOR_AUX = BMP_PRESS_VALUE;
      }

      if (ALT_BMP280_MONITOR_AUX != BMP_ALT_VALUE) {
        Serial.print("|ALT: ");
        Serial.println(BMP_ALT_VALUE);
        ALT_BMP280_MONITOR_AUX = BMP_ALT_VALUE;
      }

      if (BH1750_MONITOR_AUX != BH1750_LUX_VALUE) {
        Serial.print("|LUX: ");
        Serial.println(BH1750_LUX_VALUE);
        BH1750_MONITOR_AUX = BH1750_LUX_VALUE;
      }

      if (RTC_MONITOR_AUX != RTC_DATA_VALUE) {
        Serial.print("|RTC: ");
        Serial.println(RTC_DATA_VALUE);
        RTC_MONITOR_AUX = RTC_DATA_VALUE;
      }

      Serial.println(F("====== RITARDO ================================================="));
      Serial.println("| BH1750: " + String(BH1750_TASK_DELAY) + "ms | BMP: " + String(BMP280_TASK_DELAY) + "ms | RTC: " + String(RTC_TASK_DELAY) + "ms | TSK: " + String(TASK_MONITOR_TASK_DELAY) + "ms");
      Serial.println("=========================================| Uptime sistema: " + String(millis() / 1000) + "s");
    }
  }
}

// Loop principale (vuoto per questa applicazione)
void loop() {}
