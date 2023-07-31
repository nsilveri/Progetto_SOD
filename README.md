# Progetto Sistemi Operativi Dedicati - A.A. 2022/2023

L'obiettivo del progetto è stato quello di realizzare un sistema di acquisizioni di dati in tempo reale da diversi sensori utilizzando una Raspberry PI Zero, una Raspberry PI Pico e un broker MQTT.

Il sistema è composto dai seguenti componenti:

• Raspberry Pi Zero W
• Raspberry Pi Pico
• Sensore per la misurazione di temperatura e pressione BMP280
• Sensore per la misurazione della luminosità BH1750
• Modulo RTC

<p align="center">
  <img width="55%" height="55%" src="Immagini/Schema_progetto.png">
</p>

La scheda RPi Pico avrà il compito di acquisire le misure dai vari sensori e comunicarle alla RPi Zero mediante comunicazione I2C. Il coordinamento dei vari task che RPi Pico dovrà svolgere sarà affidato a FreeRTOS.
La RPi Zero svolgerà il compito di client MQTT e dovrà occuparsi della comunicazione dei dati
verso il broker MQTT che potrà essere implementato all’interno di un qualsiasi laptop/macchina virtuale con sistema operativo Linux.
I dati registrati all’interno del server MQTT dovranno essere visualizzabili mediante browser WEB.

La RPi Pico dovrà periodicamente leggere i dati dai sensori a sua disposizione e associare ad 
essi un timestamp ricavato mediante interrogazione del modulo RTC. I dati, così letti, dovranno essere inviati mediante comunicazione I2C alla RPi Zero, che assolve al compito di client MQTT.

La sincronizzazione del modulo RTC deve avvenire mediante un comando proveniente da RPi Zero.
Il PC (o macchina virtuale), oltre alla funzionalità di broker MQTT, dovrà dare la possibilità ai dispositivi connessi alla stessa rete di visualizzare i dati provenienti dai sensori (real time e
storico).

La repository è composta da 5 directory:

- _**Applicazione WEB**_ : contiene il frontend e backend dell'applicazione web full stack che permette di visualizzare e meorizzare sul database MongoDB tutti i dati relativi ai sensori
- _**Immagini**_ : contiene le immagini necessarie a visualizzare correttamente il file README.md
- _**mosquitto**_ : contiene i file di configurazione necessari affinché mosquitto, installato sulla macchina virtuale, sia perfettamente funzionante
- _**Raspberry Pico**_ : contiene il codice con cui è stata programmata la Raspberry PI Pico
- _**Raspberry Zero**_: contiene il codice Python caricato al suo interno sia per quanto riguarda l'MQTT che la ricezione dei dati dalla Raspberry Pico in I2C
- _**Relazione**_: contiene il PDF con la relazione che descrive nel dettaglio il funzionamento del sistema
## Inizializzazione della macchina virtuale

1) Creare una nuova macchina virtuale con DietPi al suo interno. Il link del sistema operativo si trova al sequente [link](https://dietpi.com/downloads/images/DietPi_VirtualBox-x86_64-Bookworm.7z)

2) Installare Mosquitto tramite il seguente comando:
  ```bash
  sudo apt install mosquitto
  ```
  

2) Modificare il file di configurazione di Mosquitto

```bash
sudo nano /etc/mosquitto/mosquitto.conf
```
3) Copiare il contenuto accessibile da questo [link](https://github.com/nsilveri/Progetto_SOD/blob/main/mosquitto/mosquitto.conf) e incollarlo nell'editor appena aperto. Chiudere e salvare il file attraverso CTRL+O e poi CTRL+X

4) Riavviare mosquitto
```bash
sudo systemctl restart mosquitto
``` 
6) Per verificare lo stato del servizio in qualsiasi momento
```bash
sudo systemctl status mosquitto
``` 
  
## Inizializzazione della Raspberry PI Zero

1) Scaricare DietPi per la Raspberry PI Zero al seguente [link](https://dietpi.com/downloads/images/DietPi_RPi-ARMv6-Bullseye.7z)

2) Flashare l'immagine di DietPi sulla microSD e seguire la configurazione iniziale di DietPi

3) Una volta avviata ed eseguite le configurazioni iniziali, connessa al Wi-Fi tramite il tool dedicato (dietpi-config), copiare i file nella cartella dell'utente (nel nostro caso /home/dietpi) presenti al seguente [link](https://github.com/nsilveri/Progetto_SOD/tree/main/Raspberry_Zero).

4) Installare le librerie Python tramite il tool pip: 
```bash
pip install paho-mqtt
```
5) E' necessario editare lo script main_mqtt.py andando a modificare la voce:

```bash
BROKER_ADDRESS = "<indirizzo_IP_VM>"
``` 
con l'indirizzo ip della propria macchina virtuale precedentemente creata; Nel caso si stia utilizzando Diet-Pi, l'IP viene mostrato nella schermata di avvio una volta connessi via SSH (ifconfig non è presente)

6) E' possibile avviare lo script Python o manualmente come segue:

```bash
python3 /home/dietpi/main_mqtt.py

#NOTA:
#Al primo avvio è consigliabile l'esecuzione manuale
#per controllare che non venga stampato alcun errore.
``` 

altrimenti configurare l'esecuzione automatica all'avvio del sistema operativo:
```bash
sudo nano /etc/systemd/system/main_mqtt.service
```
copiando il seguente contenuto:
```bash
[Unit]
Description=Main MQTT Service
After=network.target

[Service]
ExecStart=/usr/bin/python3 /home/dietpi/main_mqtt.py
WorkingDirectory=/home/dietpi
Restart=always

[Install]
WantedBy=multi-user.target
```
Salvando con Ctrl+X.

Infine eseguire i comandi:
```bash
sudo systemctl enable main_mqtt.service
sudo systemctl start  main_mqtt.service
```

## Inizializzazione del database MongoDB

1) Creare un nuovo database MongoDB e segnarsi il Database URI

## Inizializzazione dell'applicazione WEB

#### Inizializzazione del server NodeJS

1) Installare npm, e node con il seguente comando:

```bash
sudo apt install nodejs npm
```
2) Copiare il contenuto della cartella presente al seguente [link](https://github.com/nsilveri/Progetto_SOD/tree/main/Applicazione_WEB) all'interno della macchina virtuale con DietPi

3) Dirigersi alla cartella "backend":
```bash
cd /home/dietpi/Progetto_SOD/backend
```

4) Installare le dipendenze necessarie all'esecuzione del server NodeJS con il comando:
```bash
npm i
```

5) Modificare il file .env presente nella cartella "backend" indicando il'URI del database MongoDB:

```bash
DATABASE_URI = mongodb+srv://...
```

#### Inizializzazione del frontend in ReactJS
1) Dirigersi nella cartella del frontend:

```bash
cd /home/dietpi/Progetto_SOD/frontend
```

2) Installare le dipendenze con il comando:
```bash
npm i
```

3) Modificare il file .env presente nella cartella "frontend" indicando l'IP della macchina virtuale assegnato da DietPi

```bash
REACT_APP_HOST = <indirizzo-ip-vm>:5000
REACT_APP_MQTT_BROKER = ws://<indirizzo-ip-vm>_1884
```

#### Avvio dell'applicazione WEB

Per eseguire l'applicazione WEB, è possibile eseguire manualmente il seguente script presente all'interno della cartella "Applicazione_WEB":

```bash
sh start_web_app.sh
```
oppure si può impostare l'avvio automatico andando ad eseguire il comando:
```bash
sudo nano /etc/systemd/system/start_webapp.service
```
e copiando al suo interno le seguenti righe:
```bash
[Unit]
Description=WEB_APP Service
After=network.target

[Service]
Type=simple
ExecStart=sh /usr/local/bin/start_web_app.sh

[Install]
WantedBy=multi-user.target

```
Una volta copiato, salvare tramite Ctrl+X ed eseguire i seguenti comandi:
```bash
sudo cp start_web_app.sh /usr/local/bin
sudo systemctl enable start_webapp.service
sudo systemctl start  start_webapp.service
```
Per controllare che tutto sia andato a buon fine eseguire:
```bash
sudo systemctl status start_webapp.service
```


## Autori

- Castellucci Giacomo
- Compagnoni Paolo
- Silveri Nicola
