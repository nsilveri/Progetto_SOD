import smbus
import time
import json
import datetime
import struct
from datetime import datetime, timedelta

# RTC       = 0x13
# SYNC_TIME = 0x15

RTC = 0x45
SYNC_TIME = 0x46

reference_time = datetime.now()

class timestamp_sensor:
    def __init__(self, bus, address):
        self.address = address
        self.bus = bus

    def _write_byte(self, value):
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

    def _read_timestamp(self):
        try:
            timestamp = self.bus.read_i2c_block_data(self.address, RTC, 4)
        except Exception as e:
            print ("Read Error "+str(e))
            timestamp = False
        return timestamp
    
    def _convert_byte_to_human_ts(unix_ts):
        epoch_timestamp = (unix_ts[0] << 24) + (unix_ts[1] << 16) + (unix_ts[2] << 8) + unix_ts[3]
        dt = datetime.fromtimestamp(epoch_timestamp)
        human_readable = dt.strftime('%Y-%m-%d %H:%M:%S')
        return human_readable

    @staticmethod
    def convert_to_byte_list(data):
        byte_list = []
        for char in data:
            byte = ord(char)
            byte_list.append(byte)
        return byte_list
    
    def sync_datetime(self):
        timestamp = int(time.time())
        byte_list = timestamp.to_bytes(4, byteorder='big')
        byte_list = list(byte_list)
        
        self._write_i2c_block_data(SYNC_TIME, byte_list)
        time.sleep(0.1)

    # Function to send date and time to Arduino
    def send_datetime(self):
        current_datetime = time.strftime("%Y-%m-%d %H:%M:%S")
        byte_list = timestamp_sensor.convert_to_byte_list(current_datetime)
        for byte in byte_list:
            print(byte)
            self.bus.write_byte(self.address, byte)
            time.sleep(0.01)  # Add a small delay between bytes
        return current_datetime 

    def read_timestamp(self):
        self._write_byte(RTC)  # Write command to read timestamp
        time.sleep(0.1)  # Wait for the timestamp to be ready
        timestamp = self._read_timestamp()  # Read the timestamp
        if(timestamp != False):
            return timestamp
        return timestamp

    def generate_human_ts(self):
        RTC_DATA = self.read_timestamp()
        if(RTC_DATA):
            ts = timestamp_sensor._convert_byte_to_human_ts(RTC_DATA)
        else:
            ts = False
        return ts

    def generate_json_data(self):
        ts = timestamp_sensor.generate_human_ts(self)
        if(ts == False):
            ts = "ERROR"
        data = {
            'Name': "RTC",
            'timestamp': ts
        }

        json_data = json.dumps(data)
        return json_data
