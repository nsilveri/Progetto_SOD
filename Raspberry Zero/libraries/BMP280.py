import smbus
import time
import json

BMP_TEMP  = 0x10
BMP_PRESS = 0x11
BMP_ALT   = 0x12

class bmp_sensor:
    def __init__(self, address):
        self.address = address
        self.bus = smbus.SMBus(1)  # Use the appropriate bus number for your system

    def _write_byte(self, value):
        self.bus.write_byte(self.address, value)

    def _read_byte(self):
        return self.bus.read_byte(self.address)

    def read_temperature(self):
        self._write_byte(BMP_TEMP)  # Write command to trigger temperature measurement
        time.sleep(0.005)  # Wait for measurement to complete
        byte = self._read_byte()  # Read raw temperature data
        return byte  # Perform appropriate conversion to get actual temperature

    def read_pressure(self):
        self._write_byte(BMP_PRESS)  # Write command to trigger pressure measurement
        time.sleep(0.005)  # Wait for measurement to complete
        byte = self._read_byte()  # Read raw pressure data
        return byte  # Perform appropriate conversion to get actual pressure

    def read_altitude(self):
        self._write_byte(BMP_ALT)  # Write command to trigger altitude measurement
        time.sleep(0.005)  # Wait for measurement to complete
        byte = self._read_byte()  # Read raw altitude data
        return byte  # Perform appropriate conversion to get actual altitude
    
    def generate_json_data(self):
        TEMP  = bmp_sensor.read_temperature(self)
        PRESS = bmp_sensor.read_pressure(self)
        ALT   = bmp_sensor.read_altitude(self)
        RTC = RTC_sensor.read_timestamp()
        
        data = {
            'Name':"BMP280",
            'Temp': TEMP,
            'Press': PRESS,
            'Alt': ALT
        }

        json_data = json.dumps(data)
        return json_data


# Example usage:
'''
sensor = bmp_sensor(0x08)  # Replace with the actual address of your BMP sensor

temperature = sensor.read_temperature()
print("Temperature:", temperature)

pressure = sensor.read_pressure()
print("Pressure:", pressure)

altitude = sensor.read_altitude()
print("Altitude:", altitude)
'''

