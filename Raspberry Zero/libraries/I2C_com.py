import smbus
import time

class I2C_COM():
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

    def data_exchange(self, REG):
        self._write_byte(REG)
        time.sleep(0.5)
        byte = 99 #self._read_byte()
        return byte