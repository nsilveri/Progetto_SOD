import smbus
import time
import json

RTC = 0x13

class timestamp_sensor:
    def __init__(self, address):
        self.address = address
        self.bus = smbus.SMBus(1)  # Use the appropriate bus number for your system

    def _write_byte(self, value):
        self.bus.write_byte(self.address, value)

    def _read_timestamp(self):
        timestamp = self.bus.read_i2c_block_data(self.address, RTC, 4)
        return timestamp

    def read_timestamp(self):
        self._write_byte(RTC)  # Write command to read timestamp
        time.sleep(0.01)  # Wait for the timestamp to be ready
        timestamp = self._read_timestamp()  # Read the timestamp
        return timestamp
    
    def generate_json_data(self, address):
        RTC_DATA   = timestamp_sensor.read_timestamp()
        
        data = {
            'Name': "RTC",
            'timestamp': RTC_DATA
        }

        json_data = json.dumps(data)
        return json_data

# Example usage:
#sensor = timestamp_sensor(0x08)  # Replace with the actual address of your timestamp sensor
#
#timestamp = sensor.read_timestamp()
#print("Timestamp:", timestamp)
