import smbus
import time

BH1750_LUX = 0x13

class bh1750_sensor:
    def __init__(self, address):
        self.address = address
        self.bus = smbus.SMBus(1)  # Use the appropriate bus number for your system

    def _write_byte(self, value):
        self.bus.write_byte(self.address, value)

    def _read_word(self):
        data = self.bus.read_word_data(self.address, 0x00)
        return data

    def read_light_intensity(self):
        self._write_byte(BH1750_LUX)  # Write command to measure light intensity
        time.sleep(0.2)  # Wait for measurement to complete (adjust if needed)
        intensity = self._read_word()  # Read light intensity data
        return intensity

# Example usage:
'''
sensor = bh1750_sensor(0x08)  # Replace with the actual address of your BH1750 sensor
light_intensity = sensor.read_light_intensity()
print("Light Intensity:", light_intensity, "Lux")
'''