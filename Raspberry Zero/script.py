from libraries import BMP280, BH1750, RTC

# Indirizzo I2C della scheda Pico
PICO_ADDRESS   = 0x08

# Esempio di utilizzo
'''
RTC_REQUEST    = 0x42 #'B' #  # Messaggio da inviare al Pico

BMP280_REQUEST = 0x43 #'C' #  # Messaggio da inviare al Pico

BH1750_REQUEST = 0x44 #'D' #  # Messaggio da inviare al Pico
'''
# Inizializza il bus I2C
#bus = smbus.SMBus(1)  # Puoi utilizzare il bus 0 o 1 a seconda della tua configurazione

BMP_sensor    = BMP280.bmp_sensor(PICO_ADDRESS)
BH1750_sensor = BH1750.bh1750_sensor(PICO_ADDRESS)
RTC_sensor    = RTC.timestamp_sensor(PICO_ADDRESS)

def converti_in_stringa(sequenza):
    sequenza = str(sequenza)
    numeri = sequenza.split()
    stringa = ""
    for numero in numeri:
        stringa += chr(int(numero))
    return stringa


def BMP280_data_read():
    TEMP = BMP_sensor.read_temperature()
    #PRESS = BMP_sensor.read_pressure()
    #ALT = BMP_sensor.read_altitude()
    print('Temp: ' + str(TEMP))
    #print('Press: ' + str(PRESS))
    #print('Atl: ' + str(ALT))
'''
def send_receive_message(message):
    
    #time.sleep(0.2)
    
    response = []
    while True:
        try:
            print('==========================================')
            bus.write_byte(PICO_ADDRESS, message)

            byte = bus.read_i2c_block_data(PICO_ADDRESS, 0, 2)
            print(byte)

            #response.append(byte)
            #print(response)
            
            time.sleep(1)
            
        except IOError:
            print("Errore di comunicazione con il Pico")
            break

    return response
'''

# Esempio di utilizzo
#response = send_receive_message(BMP280_REQUEST)
#response = 
BMP280_data_read()
#print(f"Risposta dal Pico: {response}")

