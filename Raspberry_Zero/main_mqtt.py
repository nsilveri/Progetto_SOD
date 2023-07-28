#!/usr/bin/python
import paho.mqtt.client as mqtt
from paho.mqtt import client as mqtt_client
import json
from datetime import datetime
from pico_sensors import pico_sensors


BMP280_TOPIC = "BMP280"
BH1750_TOPIC = "BH1750"
RTC_TOPIC    = "RTC"
WEB_REQ_TOPIC= "WEB_REQ"
ERROR_TOPIC  = "ERROR_TOPIC"

BMP280_ID    = "BMP280_ID"
BH1750_ID    = "BH1750_ID"
RTC_ID       = "RTC_ID"
     
WEB_REQ_ID   = "WEB_REQ_ID"

CLIENT_PUBLISHER_DATA_SENDER = "PUBLISHER_DATA_SENDER"

BROKER_ADDRESS = "100.68.149.87"
BROKER_PORT=1883

client_data_sender = mqtt.Client(CLIENT_PUBLISHER_DATA_SENDER)
client_data_sender.connect(BROKER_ADDRESS, BROKER_PORT)

client_data_sender.will_set(ERROR_TOPIC,"ERROR", 0, False)

def on_disconnect(client, userdata, rc):
    if rc != 0:
        print("Connessione al broker persa. Tentativo di riconnessione...")
        client.reconnect()

client_data_sender.on_disconnect = on_disconnect

def connect_web_request() -> mqtt_client:
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Command_client connected to MQTT Broker!")
        else:
            print("Failed to connect command_client, return code %d\n", rc)

    client = mqtt_client.Client(WEB_REQ_ID)
    client.on_connect = on_connect
    client.connect(BROKER_ADDRESS, BROKER_PORT, keepalive=10)
    return client

def subscribe_WEB_REQUEST(client: mqtt_client):
    def on_message(client, userdata, msg):
        #print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")
        print(msg.payload.decode())
        data = json.loads(msg.payload.decode())
        #if isinstance(data, dict):
        on_message_command(data)

    client.subscribe([(WEB_REQ_TOPIC, 1)])

    client.on_message = on_message

    client.on_disconnect = on_disconnect

def on_message_command(data):
    print("data rec: " + str(type(data)))
    if(data["sensor"] == "BMP280"):
        DATA = pico_sensors.BMP280_data_read()
        MSG = DATA
        print("BMP280 req received")
        client_data_sender.publish(BMP280_TOPIC, MSG, 1)

    if(data["sensor"] == "BH1750"):
        DATA = pico_sensors.BH1750_data_read()
        MSG = DATA
        print("BH1750 req received")
        client_data_sender.publish(BH1750_TOPIC, MSG, 1)

    if(data["sensor"] == "RTC_READ"):
        DATA = pico_sensors.RTC_data_read()
        MSG = DATA
        client_data_sender.publish(RTC_TOPIC, MSG, 1)

    if(data["sensor"] == "RTC_SYNC"):
        DATA = pico_sensors.Sync_time_pico()
        MSG = DATA
        client_data_sender.publish(RTC_TOPIC, MSG, 1)

def run():
    client_web_request = connect_web_request()
    subscribe_WEB_REQUEST(client_web_request)
    client_web_request.loop_forever()

run()
