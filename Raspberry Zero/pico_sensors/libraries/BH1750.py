# Import delle librerie necessarie
import time
import json

# Definizione dell'indirizzo del registro per la lettura dell'intensità luminosa dal sensore BH1750
BH1750_LUX = 0x44

# Definizione della classe 'bh1750_sensor'
class bh1750_sensor:
    def __init__(self, bus, address, rtc):
        self.address = address  # Indirizzo del sensore BH1750
        self.bus = bus  # Istanza del bus I2C
        self.rtc = rtc  # Istanza del sensore RTC per ottenere il timestamp

    def _write_byte(self, value):
        try:
            self.bus.write_byte(self.address, value)  # Scrivi un byte all'indirizzo specificato
        except Exception as e:
            print("Writing Error " + str(e))

    def _write_i2c_block_data(self, reg, value):
        try:
            self.bus.write_i2c_block_data(self.address, reg, value)  # Scrivi una serie di byte all'indirizzo e registro specificati
        except Exception as e:
            print("Writing Error " + str(e))

    def _read_byte(self):
        try:
            received_data = self.bus.read_byte(self.address)  # Leggi un byte dall'indirizzo specificato
        except Exception as e:
            print("Read Error " + str(e))
            received_data = 0xab
        return received_data

    def _read_word(self):
        try:
            data = self.bus.read_word_data(self.address)  # Leggi una parola (2 byte) di dati dall'indirizzo specificato
        except Exception as e:
            print("Writing Error " + str(e))
        return data

    def _data_exchange(self, REG):
        # Funzione per scambiare dati con il sensore BH1750
        self._write_byte(REG)  # Invia il comando per la lettura dell'intensità luminosa
        time.sleep(0.1)
        byte = self._read_byte()  # Leggi il dato restituito dal sensore
        return byte

    def read_light_intensity(self):
        lux = bh1750_sensor._data_exchange(self, BH1750_LUX)  # Leggi l'intensità luminosa dal sensore BH1750
        return lux

    def generate_json_data(self, rtc):
        # Genera i dati letti dal sensore in formato JSON
        LUX = self.read_light_intensity()  # Leggi l'intensità luminosa
        RTC_DATA = rtc.generate_human_ts()  # Ottieni il timestamp dal sensore RTC

        data = {
            'lux': LUX,
            'timestamp': RTC_DATA
        }

        json_data = json.dumps(data)  # Converti i dati in formato JSON
        return json_data
