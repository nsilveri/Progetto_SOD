# Import delle librerie necessarie
import smbus
import time
import json
import datetime
import struct
from datetime import datetime, timedelta

# Definizione degli indirizzi dei registri dell'I2C per RTC e SYNC_TIME
RTC = 0x45
SYNC_TIME = 0x46

# Ottieni il riferimento temporale corrente
reference_time = datetime.now()

# Definizione della classe 'timestamp_sensor'
class timestamp_sensor:
    def __init__(self, bus, address):
        self.address = address  # Indirizzo del sensore
        self.bus = bus  # Istanza del bus I2C

    def _write_byte(self, value):
        try:
            self.bus.write_byte(self.address, value)  # Scrive un byte all'indirizzo specificato
        except Exception as e:
            print("Writing Error " + str(e))

    def _write_i2c_block_data(self, reg, value):
        try:
            self.bus.write_i2c_block_data(self.address, reg, value)  # Scrive una serie di byte all'indirizzo e registro specificati
        except Exception as e:
            print("Writing Error " + str(e))

    def _read_byte(self):
        try:
            received_data = self.bus.read_byte(self.address)  # Leggi un byte dall'indirizzo specificato
        except Exception as e:
            print("Read Error " + str(e))
            received_data = 0xab
        return received_data

    def _read_timestamp(self):
        try:
            timestamp = self.bus.read_i2c_block_data(self.address, RTC, 4)  # Leggi 4 byte dall'indirizzo e registro specificati
        except Exception as e:
            print("Read Error " + str(e))
            timestamp = False
        return timestamp

    def _convert_byte_to_human_ts(unix_ts):
        # Converte i byte di timestamp in un formato leggibile
        epoch_timestamp = (unix_ts[0] << 24) + (unix_ts[1] << 16) + (unix_ts[2] << 8) + unix_ts[3]
        dt = datetime.fromtimestamp(epoch_timestamp)
        human_readable = dt.strftime('%Y-%m-%d %H:%M:%S')
        return human_readable

    def sync_datetime(self):
        # Sincronizza il tempo dell'I2C con il tempo corrente
        timestamp = int(time.time())  # Ottieni il timestamp Unix corrente
        byte_list = timestamp.to_bytes(4, byteorder='big')  # Converti il timestamp in una lista di 4 byte
        byte_list = list(byte_list)
        self._write_i2c_block_data(SYNC_TIME, byte_list)  # Scrive il timestamp al registro di sincronizzazione
        time.sleep(0.1)

    def read_timestamp(self):
        self._write_byte(RTC)  # Invia il comando per leggere il timestamp
        time.sleep(0.1)  # Attendi che il timestamp sia pronto
        timestamp = self._read_timestamp()  # Leggi il timestamp
        if timestamp != False:
            return timestamp
        return timestamp

    def generate_human_ts(self):
        RTC_DATA = self.read_timestamp()
        if RTC_DATA:
            ts = timestamp_sensor._convert_byte_to_human_ts(RTC_DATA)  # Converte i byte del timestamp in una stringa leggibile
        else:
            ts = False
        return ts

    def generate_json_data(self):
        ts = timestamp_sensor.generate_human_ts(self)
        if ts == False:
            ts = "error"
        data = {
            'timestamp': ts
        }

        json_data = json.dumps(data)  # Converti i dati in formato JSON
        return json_data
