import time
import json
#import smbus

BH1750_LUX = 0x44

class bh1750_sensor:
    def __init__(self, bus, address, rtc):
        self.address = address
        self.bus = bus
        self.rtc = rtc
        
    def _write_byte(self, value):
        #self.bus.write_byte(self.address, value)
        try:
            self.bus.write_byte(self.address, value)
        except Exception as e:
            print ("Writing Error "+str(e))

    def _write_i2c_block_data(self, reg, value):
        try:
            self.bus.write_i2c_block_data(self.address, reg, value)
        except Exception as e:
            print ("Writing Error "+str(e))

    def _read_byte(self):
        try:
            received_data = self.bus.read_byte(self.address)
        except Exception as e:
            print ("Read Error "+str(e))
            received_data = 0xab
        return received_data

    def _read_word(self):
        try:
            data = self.bus.read_word_data(self.address)#, 0x00)
        except Exception as e:
            print ("Writing Error "+str(e))
        return data

    def _data_exchange(self, REG):
        self._write_byte(REG)
        time.sleep(0.1)
        byte = self._read_byte()
        return byte
    
    def read_light_intensity(self):
        lux = bh1750_sensor._data_exchange(self, BH1750_LUX)
        return lux
    
    def generate_json_data(self, rtc):
        #RTC_sensor = RTC_sensor.timestamp_sensor(self, address)
        LUX = self.read_light_intensity()
        RTC_DATA = rtc.generate_human_ts()
        
        data = {
            'Name': "BH1750",
            'Lux': LUX,
            'timestamp': RTC_DATA
        }

        json_data = json.dumps(data)
        return json_data

