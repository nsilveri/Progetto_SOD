#!/usr/bin/python

# Import delle librerie necessarie
import paho.mqtt.client as mqtt
from paho.mqtt import client as mqtt_client
import json
from datetime import datetime
import pico_sensors

# Definizione dei topic per i diversi sensori e richieste
BMP280_TOPIC = "BMP280"
BH1750_TOPIC = "BH1750"
RTC_TOPIC = "RTC"
WEB_REQ_TOPIC = "WEB_REQ"
ERROR_TOPIC = "ERROR_TOPIC"

# Definizione degli ID per i diversi client
BMP280_ID = "BMP280_ID"
BH1750_ID = "BH1750_ID"
RTC_ID = "RTC_ID"
WEB_REQ_ID = "WEB_REQ_ID"

# Definizione dell'ID del client per l'invio dei dati
CLIENT_PUBLISHER_DATA_SENDER = "PUBLISHER_DATA_SENDER"

# Indirizzo e porta del broker MQTT
BROKER_ADDRESS = "100.68.149.87"  # Inserire l'indirizzo IP del broker MQTT
BROKER_PORT = 1883

# Creazione del client MQTT per l'invio dei dati
client_data_sender = mqtt.Client(CLIENT_PUBLISHER_DATA_SENDER)
client_data_sender.connect(BROKER_ADDRESS, BROKER_PORT)

# Configurazione del "will" per gestire l'eventuale disconnessione del client
client_data_sender.will_set(ERROR_TOPIC, "ERROR", 0, False)

# Funzione per connettersi al client MQTT per le richieste web
def connect_web_request() -> mqtt_client:
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Command_client connected to MQTT Broker!")
        else:
            print("Failed to connect command_client, return code %d\n", rc)

    client = mqtt_client.Client(WEB_REQ_ID)
    client.on_connect = on_connect
    client.connect(BROKER_ADDRESS, BROKER_PORT, keepalive=60)
    return client

# Funzione per iscriversi al topic delle richieste web
def subscribe_WEB_REQUEST(client: mqtt_client):
    def on_message(client, userdata, msg):
        print(msg.payload.decode())
        data = json.loads(msg.payload.decode())
        on_message_command(data)

    client.subscribe([(WEB_REQ_TOPIC, 0)])
    client.on_message = on_message

# Funzione per gestire il messaggio ricevuto dalle richieste web
def on_message_command(data):
    print("data rec: " + str(type(data)))
    if data["sensor"] == "BMP280":
        DATA = pico_sensors.BMP280_data_read()  # Leggi i dati dal sensore BMP280
        MSG = DATA
        print("BMP280 req received")
        client_data_sender.publish(BMP280_TOPIC, MSG, 1)  # Pubblica i dati sul topic relativo al sensore BMP280

    if data["sensor"] == "BH1750":
        DATA = pico_sensors.BH1750_data_read()  # Leggi i dati dal sensore BH1750
        MSG = DATA
        print("BH1750 req received")
        client_data_sender.publish(BH1750_TOPIC, MSG, 1)  # Pubblica i dati sul topic relativo al sensore BH1750

    if data["sensor"] == "RTC_READ":
        DATA = pico_sensors.RTC_data_read()  # Leggi i dati dal sensore RTC
        MSG = DATA
        client_data_sender.publish(RTC_TOPIC, MSG, 1)  # Pubblica i dati sul topic relativo al sensore RTC

    if data["sensor"] == "RTC_SYNC":
        DATA = pico_sensors.Sync_time_pico()  # Sincronizza il tempo con il sensore RTC
        MSG = DATA
        client_data_sender.publish(RTC_TOPIC, MSG, 1)  # Pubblica i dati sul topic relativo al sensore RTC

# Funzione principale per eseguire il programma
def run():
    client_web_request = connect_web_request()  # Connettiti al client per le richieste web
    subscribe_WEB_REQUEST(client_web_request)  # Iscriviti al topic delle richieste web
    client_web_request.loop_forever()  # Avvia il loop del client MQTT per le richieste web

run()  # Esegui la funzione principale per avviare il programma
