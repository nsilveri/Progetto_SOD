import time
import json
#import smbus

# BH1750_LUX = 0x13
BH1750_LUX = 0x44


class bh1750_sensor:
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

    def _read_word(self):
        data = self.bus.read_word_data(self.address)#, 0x00)
        return data

    def read_light_intensity(self):
        self._write_byte(BH1750_LUX) 
        time.sleep(0.2) 
        #intensity = self._read_word() 
        intensity = self._read_byte()
        return intensity
    
    def generate_json_data(self, address): #, RTC):
        #RTC_sensor = RTC_sensor.timestamp_sensor(self, address)
        LUX = self.read_light_intensity()
        #RTC_DATA = RTC.read_timestamp()
        
        data = {
            'Name': "BH1750",
            'Lux': LUX#,
            #'timestamp': RTC_DATA
        }

        json_data = json.dumps(data)
        return json_data

