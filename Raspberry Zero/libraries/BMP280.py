import time
import json
import smbus

BMP_TEMP = 0x41
BMP_PRESS = 0x42
BMP_ALT = 0x43


class bmp_sensor:
    def __init__(self, bus, address):
        self.address = address
        self.bus = bus

    def _write_byte(self, value):
        #self.bus.write_byte(self.address, value)
        try:
            self.bus.write_byte(self.address, value)
        except Exception as e:
            print ("Writing Error "+str(e))

    def _write_i2c_block_data(self, reg, value):
        self.bus.write_i2c_block_data(self.address, reg, value)

    def _read_byte(self):
        try:
            received_data = self.bus.read_byte(self.address)
        except Exception as e:
            print ("Read Error "+str(e))
            received_data = 0xab
        return received_data

    def _data_exchange(self, REG):
        self._write_byte(REG)
        time.sleep(0.5)
        byte = 99 #self._read_byte()
        return byte

    def read_temperature(self):
        temperature = bmp_sensor._data_exchange(self, BMP_TEMP)
        return temperature

    def read_pressure(self):
        pressure = bmp_sensor._data_exchange(self, BMP_PRESS)
        return pressure

    def read_altitude(self):
        altitude = bmp_sensor._data_exchange(self, BMP_ALT)
        return altitude
    
    def generate_json_data(self, address):
        # RTC_sensor = RTC.timestamp_sensor(address)
        TEMP = self.read_temperature()
        time.sleep(0.1)
        PRESS = self.read_pressure()
        time.sleep(0.1)
        ALT = self.read_altitude()
        # RTC_DATA   = RTC_sensor.read_timestamp()

        data = {
            # 'timestamp': RTC_DATA,
            'Name': 'BMP280',
            'Temp': TEMP,
            'Press': PRESS,
            'Alt': ALT 
        }

        json_data = json.dumps(data)
        return json_data
