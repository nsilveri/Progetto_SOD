import smbus
import time
import json
import datetime

# RTC       = 0x13
# SYNC_TIME = 0x15

RTC = 0x45
SYNC_TIME = 0x46

class timestamp_sensor:
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

    def _read_timestamp(self):
        timestamp = self.bus.read_i2c_block_data(self.address, RTC, 4)
        print(timestamp)
        readable_timestamp = datetime.datetime.fromtimestamp(int.from_bytes(timestamp, 'big')).strftime('%Y-%m-%d %H:%M:%S')
        print(readable_timestamp)
        return readable_timestamp

    @staticmethod
    def convert_to_byte_list(data):
        byte_list = []
        for char in data:
            byte = ord(char)
            byte_list.append(byte)
        return byte_list

    def sync_datetime(self):
        current_datetime = time.strftime("%Y-%m-%d %H:%M:%S")
        timestamp = int(time.time())
        byte_list = timestamp.to_bytes(4, byteorder='big')  # Assuming a 4-byte timestamp
        byte_list = list(byte_list)

        self._write_i2c_block_data(SYNC_TIME, byte_list)
        time.sleep(0.1)

    # Function to send date and time to Arduino
    def send_datetime(self):
        # Get the current date and time
        current_datetime = time.strftime("%Y-%m-%d %H:%M:%S")
        # Send the date and time to the Arduino byte by byte
        byte_list = timestamp_sensor.convert_to_byte_list(current_datetime)
        for byte in byte_list:
            self.bus.write_byte(self.address, byte)
            time.sleep(0.01)  # Add a small delay between bytes

    def read_timestamp(self):
        self._write_byte(RTC)  # Write command to read timestamp
        time.sleep(0.01)  # Wait for the timestamp to be ready
        timestamp = self._read_timestamp()  # Read the timestamp
        return timestamp

    def generate_json_data(self, address):
        RTC_DATA = self.read_timestamp()

        data = {
            'Name': "RTC",
            'timestamp': RTC_DATA
        }

        json_data = json.dumps(data)
        return json_data
