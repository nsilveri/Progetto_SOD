# Import delle librerie custom per i sensori BMP280, BH1750 e RTC
from libraries import BMP280, BH1750, RTC
import json
import time
import smbus

# Indirizzo I2C della scheda Pico
PICO_ADDRESS = 0x08
I2C_BUS_NUMBER = 0

# Inizializzazione del bus I2C
I2C_STATE = False

while I2C_STATE == False:
    try:
        bus = smbus.SMBus(I2C_BUS_NUMBER)  # Inizializza il bus I2C
        I2C_STATE = True
        print("Setting I2C...")
        time.sleep(1)
    except:
        print("I2C Error")
        I2C_STATE = False
        time.sleep(0.5)

# Creazione delle istanze dei sensori RTC, BMP280 e BH1750
RTC_sensor = RTC.timestamp_sensor(bus, PICO_ADDRESS)  # Sensore RTC per gestire il timestamp
BMP_sensor = BMP280.bmp_sensor(bus, PICO_ADDRESS, RTC_sensor)  # Sensore BMP280 per la temperatura, pressione e altitudine
BH1750_sensor = BH1750.bh1750_sensor(bus, PICO_ADDRESS, RTC_sensor)  # Sensore BH1750 per la luminosità

def BMP280_data_read():
    DATA = BMP_sensor.generate_json_data(RTC_sensor)  # Leggi i dati dal sensore BMP280 e genera il JSON
    print(DATA)
    return DATA

def BH1750_data_read():
    DATA = BH1750_sensor.generate_json_data(RTC_sensor)  # Leggi i dati dal sensore BH1750 e genera il JSON
    print(DATA)
    return DATA

def RTC_data_read():
    DATA = RTC_sensor.generate_json_data()  # Genera il JSON con il timestamp dal sensore RTC
    print(DATA)
    return DATA

def Sync_time_pico():
    RTC_sensor.sync_datetime()  # Sincronizza il tempo con il sensore RTC
    return "RTC sync successful!"

Sync_time_pico()  # Esegui la sincronizzazione del tempo con il sensore RTC
