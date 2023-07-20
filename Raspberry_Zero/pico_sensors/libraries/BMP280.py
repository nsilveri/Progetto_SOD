# Import delle librerie necessarie
import time
import json
import smbus

# Definizione degli indirizzi dei registri per la lettura dei dati dal sensore BMP280
BMP_TEMP = 0x41
BMP_PRESS = 0x42
BMP_ALT = 0x43

# Definizione della classe 'bmp_sensor'
class bmp_sensor:
    def __init__(self, bus, address, rtc):
        self.address = address  # Indirizzo del sensore BMP280
        self.bus = bus  # Istanza del bus I2C
        self.rtc = rtc  # Istanza del sensore RTC per ottenere il timestamp

    def _write_byte(self, value):
        try:
            self.bus.write_byte(self.address, value)  # Scrivi un byte all'indirizzo specificato
        except Exception as e:
            print("Writing Error " + str(e))

    def _write_i2c_block_data(self, reg, value):
        self.bus.write_i2c_block_data(self.address, reg, value)  # Scrivi una serie di byte all'indirizzo e registro specificati

    def _read_byte(self):
        try:
            received_data = self.bus.read_byte(self.address)  # Leggi un byte dall'indirizzo specificato
        except Exception as e:
            print("Read Error " + str(e))
            received_data = 0xab
        return received_data

    def _data_exchange(self, REG):
        # Funzione per scambiare dati con il sensore BMP280
        self._write_byte(REG)  # Invia il comando per la lettura del dato specifico (temperatura, pressione o altitudine)
        time.sleep(0.1)
        byte = self._read_byte()  # Leggi il dato restituito dal sensore
        return byte

    def read_temperature(self):
        temperature = bmp_sensor._data_exchange(self, BMP_TEMP)  # Leggi la temperatura dal sensore BMP280
        return temperature

    def read_pressure(self):
        pressure = bmp_sensor._data_exchange(self, BMP_PRESS)  # Leggi la pressione dal sensore BMP280
        return pressure

    def read_altitude(self):
        altitude = bmp_sensor._data_exchange(self, BMP_ALT)  # Leggi l'altitudine dal sensore BMP280
        return altitude

    def generate_json_data(self, rtc):
        # Genera i dati letti dai sensori in formato JSON
        TEMP = self.read_temperature()  # Leggi la temperatura
        PRESS = self.read_pressure()  # Leggi la pressione
        ALT = self.read_altitude()  # Leggi l'altitudine
        RTC_DATA = rtc.generate_human_ts()  # Ottieni il timestamp dal sensore RTC

        data = {
            'temperature': TEMP,
            'pressure': PRESS,
            'altitude': ALT,
            'timestamp': RTC_DATA
        }

        json_data = json.dumps(data)  # Converti i dati in formato JSON
        return json_data
