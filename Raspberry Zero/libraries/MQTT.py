import paho.mqtt.client as mqtt

BROKER_ADDRESS = "127.0.0.1"

# Funzione di callback richiamata quando il client si connette al broker MQTT
def on_connect(client, userdata, flags, rc):
    print("Connesso al broker MQTT con codice di risultato: " + str(rc))
    # Iscrizione a un topic di esempio
    client.subscribe("topic/esempio")

# Funzione di callback richiamata quando il client riceve un messaggio MQTT
def on_message(client, userdata, msg):
    print("Messaggio ricevuto dal topic: " + msg.topic + " - Contenuto: " + str(msg.payload.decode()))

# Creazione di un oggetto client MQTT
client = mqtt.Client()

# Impostazione delle funzioni di callback
client.on_connect = on_connect
client.on_message = on_message

# Connessione al broker MQTT
broker_address = BROKER_ADDRESS
port = 1883
client.connect(broker_address, port)

# Avvio del loop di gestione delle comunicazioni MQTT
client.loop_start()

# Esecuzione di altre operazioni o attesa
# Ad esempio, puoi mettere il programma in attesa per 5 secondi
import time
time.sleep(5)

# Interruzione del loop di gestione delle comunicazioni MQTT
client.loop_stop()
