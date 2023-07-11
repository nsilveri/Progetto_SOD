from libraries import BMP280, BH1750, RTC
import json
import time
import smbus

# I2C address of the Pico board
PICO_ADDRESS = 0x08
I2C_BUS_NUMBER = 0

# Initialize the I2C bus
I2C_STATE = False

while I2C_STATE == False:
    try:
        bus = smbus.SMBus(I2C_BUS_NUMBER)
        I2C_STATE = True
        print("Setting I2C...")
        time.sleep(1)
    except:
        print("I2C Error")
        I2C_STATE = False
        time.sleep(0.5)

BMP_sensor    = BMP280.bmp_sensor(bus, PICO_ADDRESS)
BH1750_sensor = BH1750.bh1750_sensor(bus, PICO_ADDRESS)
RTC_sensor    = RTC.timestamp_sensor(bus, PICO_ADDRESS)


def convert_to_string(sequence):
    sequence = str(sequence)
    numbers = sequence.split()
    string = ""
    for number in numbers:
        string += chr(int(number))
    return string


def BMP280_data_read():
    DATA = BMP_sensor.generate_json_data(PICO_ADDRESS)
    print(DATA)
    return DATA


def BH1750_data_read():
    DATA = BH1750_sensor.generate_json_data(PICO_ADDRESS) #, RTC_sensor)
    print(DATA)
    return DATA


def RTC_data_read():
    DATA = RTC_sensor.generate_json_data(PICO_ADDRESS)
    print(DATA)
    return DATA


def Sync_time_pico():
    RTC_sensor.sync_datetime()


def main():

    while True:
        print("1. Read BMP280 data")
        print("2. Read BH1750 data")
        print("3. Read RTC data")
        print("4. Sync Pico time")
        print("5. Exit")

        choice = input("Enter your choice: ")

        if choice == "1":
            BMP280_data_read()
        elif choice == "2":
            BH1750_data_read()
        elif choice == "3":
            RTC_data_read()
        elif choice == "4":
            Sync_time_pico()
        elif choice == "5":
            print("Exiting...")
            break
        else:
            print("Invalid choice. Please try again.")


main()
