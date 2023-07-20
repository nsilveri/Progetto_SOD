
![](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.001.png)<a name="_hlk140571406"></a> 





Università Politecnica delle Marche

Dipartimento di Ingegneria dell’Informazione

Rete di Sensori mediante Pi Pico e FreeRTOS



Paolo Compagnoni, Nicola Silveri, Giacomo Castellucci

*Docenti:* Daniele Marcozzi

Corso di Laurea Magistrale in Ingegneria Informatica e dell’Automazione

Progetto di Sistemi operativi dedicati


Dichiarazione

Noi, Paolo Compagnoni, Nicola Silveri, Giacomo Castellucci del Dipartimento di Ingegneria dell’informazione, Università Politecnica delle Marche, confermiamo che questo lavoro è frutto delle nostre ricerche e che le figure, le tabelle, le equazioni, i frammenti di codice, e le illustrazioni contenute in questo rapporto sono originali e non sono state prese dal lavoro di altre persone, tranne quando le opere di altri sono state esplicitamente riconosciute, citate, e referenziate. Capiamo che in caso contrario sarà considerato un caso di plagio. Il plagio è una forma di cattiva condotta accademica e sarà penalizzato di conseguenza.

Diamo il consenso alla condivisione di una copia della nostra relazione per essere condivisa con studenti futuri come esempio di progetto.

Paolo Compagnoni

Nicola Silveri

Giacomo Castellucci

14 luglio 2023


















# Sommario
[Introduzione	4](#_toc140742973)

[Struttura Hardware	6](#_toc140742974)

[Raspberry Pi Pico:	6](#_toc140742975)

[Raspberry Pi Zero:	10](#_toc140742976)

[Connessioni Hardware:	12](#_toc140742977)

[Bus I2C:	13](#_toc140742978)

[Trasferimento dati sul Bus i2C :	14](#_toc140742979)

[Libreria SmBus :	15](#_toc140742980)

[Struttura Software	17](#_toc140742981)

[Raspberry Pi Pico	17](#_toc140742982)

[Porting della piattaforma RP2040 su Arduino	17](#_toc140742983)

[Implementazione protocollo I2C su Raspberry Pico	18](#_toc140742984)

[Libreria Wire.h :	18](#_toc140742985)

[FreeRTOS	20](#_toc140742986)

[FreeRTOS nel nostro progetto	27](#_toc140742987)

[Raspberry Pi Zero W	32](#_toc140742988)

[Sistema operativo utilizzato:	32](#_toc140742989)

[Diet-Pi	32](#_toc140742990)

[Implementazione nel progetto	33](#_toc140742991)

[Codice	34](#_toc140742992)

[Applicazione web full stack	35](#_toc140742993)

[Mongo DB	36](#_toc140742994)

[Backend	36](#_toc140742995)

[Frontend	36](#_toc140742996)




Elenco delle figure

1\.1 	Rappresentazione del sistema implementato ............................................................................. 4

2\.1	 Pinout Raspberry PI Pico …........................................................................................................... 6

2\.2 Pinout Raspberry PI Zero W. ...................................................................................................... 10

2\.3	 Struttura del Bus I2C single-master multi-slave ......................................................................... 12

2\.4 Tipica sequenza di trasferimento dati con il protocollo I2C........................................................ 13

2\.5	 Rappresentazione delle connessioni effettuate…....................................................................... 19 	  
# <a name="_toc2095429374"></a><a name="_toc140742973"></a>Introduzione
![](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.002.png)![](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.003.png)

**Figura 1.1:** Rappresentazione del sistema implementato** 

Questo progetto consiste nella realizzazione di un sistema embedded che acquisisce ciclicamente dati da tre sensori, BMP280 (temperatura, pressione, altitudine), BH1750 (luminosità) ed un RTC (Timestamp). Successivamente su richiesta pubblica mediante MQTT, i dati dei relativi sensori vengono trasferiti su un BROKER ospitato su una macchina remota (una Virtual Machine nel nostro caso) che è in grado di fare anche da server per ospitare una pagina web dove, uno o più utenti, possono interagire per la visualizzazione sia dei dati in real time, sia dello storico dei suddetti sensori memorizzati in un database MongoDB.

Possiamo suddividere il sistema in tre macro-blocchi:

1. **Acquisizione dati:** è costituito dalla scheda **Raspberry Pi Pico** che si occupa della richiesta dei dati ai sensori e del trasferimento dei dati a **Pi Zero W**. Pi Pico instaurerà quindi due comunicazioni i2c separate. Nella comunicazione con i sensori si presenta come Master mentre nella comunicazione con Pi Zero W si comporta come Slave.                                           Per quanto riguarda la programmazione di Pi Pico, è stato implementato il porting di Arduino per Pi Pico, potendo così implementare **FreeRTOS** per il coordinamento dei Task.

1. **Trasferimento dati:** è costituito da una Raspberry Pi Zero W, con installata la distribuzione ***Diet-Pi***,* che si occupa della richiesta dei dati, della gestione delle richieste ricevute via MQTT da parte dell’applicazione web e rispondendo alle stesse con i dati dei singoli sensori richiesti.

1. **Supervisione e Server:** è costituito da un’applicazione web full stack sviluppata su PC con macchina virtuale Linux, su cui viene implementato il Brooker MQTT. Per poter gestire il salvataggio dei dati è stato utilizzato il database MongoDB. Lato backend, è stato implementato un server in NodeJS per gestire la connessione con il database MongoDB e la gestione dei dati storici dei sensori. Per quanto riguarda la visualizzazione dei dati è stata creata una pagina WEB full stack mediante l’utilizzo del framework ReactJS che permette di visualizzare sia lo storico dei dati, mediante grafici temporali, sia di richiedere i dati RealTime mediante un bottone apposito.














# <a name="_toc1218771813"></a><a name="_toc140742974"></a>Struttura Hardware
In questa sezione, vengono riportati i pinout dei device utilizzati e il protocollo di comunicazione implementato.
### <a name="_toc1107109343"></a><a name="_toc140742975"></a>Raspberry Pi Pico:
![](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.004.png)

**Figura 2.1:** Pinout Raspberry PI Pico 

La Raspberry Pi Pico è un microcontrollore a basso costo, prodotta dalla Raspberry Pi Foundation. È stata progettata per fornire un'opzione economica per progetti embedded e per l'apprendimento delle basi della programmazione e dell'elettronica.

Per quanto riguarda l’hardware di bordo, troviamo una CPU ***RP2040***, sviluppato internamente dalla Raspberry Pi Foundation, dual-core ARM Cortex-M0+, dotato di una velocità di clock di 133 MHz e dispone di 264 KB di memoria RAM. È in grado di gestire una varietà di attività computazionali e di controllo in tempo reale. La Pico ha un *form factor* compatto, simile alla controparte Arduino, e offre una serie di connettori GPIO (General Purpose Input/Output) che consentono di collegare sensori, attuatori e altri dispositivi esterni. Questi connettori GPIO possono essere programmati per svolgere varie funzioni, offrendo flessibilità nella creazione di progetti personalizzati.


**Ambienti di sviluppo**

La Pico offre tre possibilità per quanto riguarda la programmazione:

- **MicroPython**
  - un interprete di Python ottimizzato per microcontrollori che rende la Pico molto accessibile ai principianti e consente di scrivere rapidamente codice per controllare dispositivi e realizzare progetti senza la necessità di imparare linguaggi di programmazione più complessi.
- **PicoSDK**
  - Con PicoSDK gli sviluppatori possono scrivere codice in C/C++ per controllare l'hardware della Raspberry Pi Pico, sfruttando le sue funzionalità e personalizzandole per i propri progetti. Il PicoSDK fornisce una base solida per lo sviluppo di applicazioni embedded, l'apprendimento delle basi dell'elettronica e l'esplorazione dell'Internet of Things (IoT) utilizzando la Pico come piattaforma di sviluppo.
- **Arduino**
  - Oltre alle due soluzioni fornite dalla stessa Raspberry Pi Foundation, è possibile sviluppare sulla Pico utilizzando la classica piattaforma Arduino.

Per il supporto alla piattaforma di sviluppo Arduino, abbiamo due soluzioni: 

- **Mbed OS**
  - Sistema operativo open-source, con obiettivi del tutto simili a quelli di **FreeRTOS,** sviluppato dalla società Arm che fornisce un'ampia gamma di funzionalità e componenti per dispositivi con restrizioni di risorse come microcontrollori e microprocessori. Mbed OS è progettato per semplificare lo sviluppo di applicazioni IoT (Internet of Things) e offre un set di librerie e API per la gestione delle periferiche hardware, la connettività di rete e altre funzionalità comuni.
- **Arduino-Pico**:
  - Si tratta di un porting sviluppato da Earle F. Philhower III che fornisce un completo supporto per la Pico nell'ambiente di sviluppo Arduino IDE. Questa libreria consente agli sviluppatori di utilizzare l'IDE Arduino per programmare la Pico, sfruttando le funzionalità e le risorse offerte dalla piattaforma Arduino attraverso cui è possibile utilizzare tutte le funzionalità della Pico, come i suoi pin GPIO, le comunicazioni seriali, gli *interrupt* e altro ancora, attraverso le comuni chiamate di funzione e le API fornite dall'ambiente Arduino. Inoltre, la libreria fornisce un'interfaccia semplice per l'accesso alle funzioni avanzate della Pico, consentendo agli sviluppatori di sfruttare appieno le potenzialità della scheda.

**General Purpose Input Output (GPIO)**

- È un'interfaccia presente in molti dispositivi elettronici, tra cui la Pico. Essa permette di collegare e controllare dispositivi esterni come sensori, attuatori e altri componenti elettronici.

La **GPIO** è costituita da un insieme di **pin** (o piedini) che possono essere configurati come input o output digitali e analogici. Un pin di input può essere utilizzato per leggere lo stato di un segnale esterno, come ad esempio il valore di un sensore. Un **pin** di output, invece, può essere utilizzato per inviare un segnale di controllo ad un dispositivo esterno, come l'accensione di un LED o il controllo di un motore.

La **Pico**, in particolare, dispone di 26 pin GPIO (0-26) che offrono diverse funzionalità e possibilità di configurazione.

Ad esempio:

- **Pin GPIO 0-26**
  - Questi pin possono essere configurati come input o output digitali. Possono essere utilizzati per leggere segnali di input da sensori esterni o per inviare segnali di controllo a dispositivi esterni come LED, motori e relè.
- **Pin analogici**
  - Alcuni pin GPIO (0-26) possono essere configurati come pin analogici, che consentono di leggere segnali analogici provenienti da sensori come potenziometri, termistori o fotocellule.
- **UART**
  - dispone di due coppie di pin (GPIO 0-1 e GPIO 16-17) che possono essere utilizzati per la comunicazione seriale asincrona (UART). Questi pin possono essere collegati a dispositivi esterni come moduli Bluetooth o convertitori USB-Serial per la trasmissione e la ricezione di dati.
- **I2C**
  - supporta la comunicazione I2C tramite due coppie di pin (GPIO 0-1 come SDA e SCL e GPIO 8-9 come SDA e SCL alternative). Questi pin possono essere utilizzati per collegare la Pico a dispositivi come sensori di temperatura, accelerometri o EEPROM.
- **SPI**
  - La Pico dispone di due coppie di pin (GPIO 0-3 come SCK, MOSI, MISO e GPIO 16-19 come SCK, MOSI, MISO alternative) che possono essere utilizzate per la comunicazione seriale sincrona (SPI). Questi pin possono essere collegati a dispositivi come display TFT, sensori di pressione o schede di memoria SD.
- **PWM**
  - pin GPIO (0-19) che supportano la generazione di segnali PWM (Pulse Width Modulation). Questi pin possono essere utilizzati per controllare la luminosità di un LED, la velocità di un motore o la gestione di segnali analogici simulati.

**ALIMENTAZIONE**

- La Pico può essere alimentata in due modi principali:
  - **MicroUSB**
    - La Pico può essere alimentata in modo semplice e conveniente tramite un cavo MicroUSB collegato a un computer o a qualsiasi altra fonte di alimentazione USB. Questo include adattatori di alimentazione USB, power bank o porte USB disponibili su altri dispositivi. Utilizzando un cavo MicroUSB standard, è possibile fornire energia alla Pico in modo pratico e senza necessità di componenti aggiuntivi. Basta collegare il cavo MicroUSB all'apposito connettore sulla scheda per alimentarla.
  - **PIN VSYS**
    - ` `La Pico offre anche un pin dedicato chiamato "**VSYS**" che può essere utilizzato per l'alimentazione. 

Questo pin consente di fornire energia alla Pico tramite una fonte di alimentazione esterna, come una batteria o un alimentatore dedicato. Collegando la tensione adeguata all'ingresso "**VSYS**" tramite connessioni adeguate, è possibile alimentare la Pico in modo indipendente, senza utilizzare il cavo MicroUSB.

La flessibilità di alimentazione offerta dalla Pico consente di adattarla alle specifiche esigenze del progetto. Sia che si stia sviluppando un prototipo collegato al computer tramite USB o un dispositivo autonomo con alimentazione esterna, la Pico offre opzioni pratiche e versatili per l'alimentazione.

**Raspberry Pico nel nostro progetto**

- Nel nostro progetto, tra le capacità della Pico sono stati utilizzati i due bus I2C, **I2C0 (PIN 4, 5)** per la comunicazione in modalità **Slave** con la Raspberry Zero ed **I2C1 (PIN 3, 4)**, in modalità **Master**, per la comunicazione con i sensori utilizzati.
















### <a name="_toc379212472"></a><a name="_toc140742976"></a>Raspberry Pi Zero:

![](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.005.png)

**Figura 2.2:** Pinout Raspberry PI Zero W



La **Raspberry Pi Zero W** è una versione compatta e a basso costo della popolare famiglia di schede Raspberry Pi. La "**W**" nel nome sta per "Wireless" (senza fili), che indica la presenza di connettività **Wi-Fi** e **Bluetooth** integrata sulla scheda.

Per quanto riguarda l’hardware di bordo, ci troviamo di fronte ad una **CPU** single-core **ARM11 Broadcom BCM2835** con clock a **1 GHz,** **512 MB** di memoria **RAM** ed il supporto per la connettività **Wi-Fi 802.11b/g/n** e **Bluetooth 4.1**. Ciò consente di connettere la scheda a reti wireless e di comunicare con dispositivi compatibili tramite Bluetooth. 

Anche se è meno potente rispetto ad altre schede Raspberry Pi, offre ancora prestazioni adeguate a molti progetti embedded in cui sono richiesti un’occupazione di spazio piccolo ed un consumo energetico basso.


**Porte e connettori**

- La Raspberry Pi Zero W dispone di una serie di connettori e porte, tra cui:
  - **Mini HDMI**
    - per collegare la scheda a un monitor o a una TV.
  - **Micro USB**
    - per l'alimentazione e per la connessione a dispositivi esterni come mouse e tastiera.
  - **Connettore per la fotocamera**
    - per collegare una fotocamera Raspberry Pi e acquisire immagini o video.
  - **Connettore per il display**
    - per collegare un display touchscreen compatibile.
  - **GPIO**
    - Come precedentemente descritto per la Pico, anche la Zero W una **GPIO** con ben **40 pin** che consentono di collegare sensori, attuatori e altri dispositivi esterni.

Rispetto alla Pico, la Raspberry Pi Zero W non dispone di pin GPIO analogici come la Pico. I pin GPIO sulla Zero sono tutti di tipo digitale, il che significa che possono essere utilizzati solo per segnali digitali (**ON/OFF**), di conseguenza non è possibile leggere direttamente segnali analogici da sensori o generare segnali analogici tramite i pin GPIO sulla Zero W.

Sistema operativo: La Raspberry Pi Zero W è compatibile con una vasta gamma di sistemi operativi, tra cui il sistema operativo Linux Raspbian, che è l'opzione consigliata dalla Raspberry Pi Foundation.














### <a name="_toc140742977"></a>Connessioni Hardware:

**In figura 2.5** vengono riportare le connessioni delle due comunicazioni i2c implementate. La comunicazione I2c0 per lo scambio di dati tra Raspberry Pi PIco e Raspberry PI Zero. E la comunicazione I2c1 per la lettura dei dati dei sensori i cui indirizzi sono:

BMP280           =   0x76,                                                                                                                                        BH1750            =   0x23,
RTC(DS1307)   =   0x68.

![](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.006.png)

`                        `**Figura 2.5:** Rappresentazione delle connessioni effettuate; **i2C0\_SDA** (giallo) **i2C0\_SCL** (verde), **i2C1\_SDA** (arancione), **i2C1\_SCL** (azzurro), **GND** (nero).







### <a name="_toc166542281"></a><a name="_toc140742978"></a>Bus I2C:

Il protocollo **I2C** (pronuncia I-quadro-C, in Inglese I-squared-C) è stato creato dalla Philips Semiconductors nel 1982; la sigla, comunemente indicata anche con I2C, sta per Inter-Integrated Circuit. Il protocollo permette la comunicazione di dati tra due o più dispositivi I2C utilizzando un bus a due fili, più uno per il riferimento comune di tensione (Figura 2.3). In tale protocollo le informazioni sono inviate serialmente usando una linea per i dati (SDA: Serial Data line) ed una per il Clock (SCL: Serial Clock line). Deve inoltre essere presente una terza linea: la massa, comune a tutti i dispositivi.                                                                                                                                      Come si può osservare dallo schema, sono presenti due resistenze di pull-up connesse tra la rete bifilare e l’alimentazione. Lo scopo di queste resistenze è di evitare un valore fluttuante dei due segnali, impedendo ai vari dispositivi di male interpretare gli eventuali disturbi sul bus. In questo modo si garantisce ai dispositivi connessi un segnale debolmente alto sulle due linee. Nei sensori con comunicazione i2C, più recenti, presenti sul mercato le resistenze di pull up vengono già integrate dal costruttore o in altri casi possono essere attivate via software sulle schede per sistemi embedded come Raspberry Pi Zero e Pi Pico sui pinout dedicati alla comunicazione i2C. 

![](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.007.jpeg)

**Figura 2.3:** Struttura del Bus I2C single-master multi-slave.

La struttura più semplice di un sistema I2C è composta da un **Master** ed uno **Slave.** Il dispositivo master è semplicemente il dispositivo che controlla il bus in un certo istante; tale dispositivo controlla il segnale di Clock e genera i segnali di START e di STOP. I dispositivi slave semplicemente “ascoltano” il bus ricevendo dati dal master o inviandone qualora questo ne faccia loro richiesta.







### <a name="_toc1866884508"></a><a name="_toc140742979"></a>Trasferimento dati sul Bus i2C :

**SIMBOLI CHIAVE DEL BUS I2C:**

|**S**|Start  (condizione iniziale)|
| :- | :- |
|**Sr**|Condizione di avvio ripetuta, utilizzata per passare dalla modalità di scrittura a quella di lettura.|
|**P**|Stop condition|
|**Rd/Wr (1 bit)**|Read/Write bit. Rd valore 1, Wr valore 0.|
|**A, NA (1 bit)**|Acknowledge (ACK) e Not Acknowledge (NACK) bit|
|**Addr (7 bits)**|I2C 7 bit address. Questo indirizzo può essere espanso fino ad ottenere un indirizzo I2C a 10 bit.|
|**Comm (8 bits)**|Command byte. Un byte di dati che spesso seleziona un registro sul dispositivo.|
|**Data (8 bits)**|Un semplice byte di dati. DataLow e DataHigh rappresentano il byte Low e High di una parola a 16 bit.|
|**Count (8 bits)**|Un byte di dati contenente la lunghezza di un'operazione di blocco.|

![](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.008.png)

**Figura 2.4:** Tipica sequenza di trasferimento dati con il protocollo I2C.

Una sequenza elementare di lettura o scrittura di dati tra master e slave segue il seguente ordine:

1\. Invio del bit di START (S) da parte del master; 

2\. Invio dell’indirizzo dello slave (ADDR) di 7 bit ad opera del master;

3\. Invio del bit di Read (R) o di Write (W), che valgono rispettivamente 1 e 0 (sempre ad opera del master); 

4\. Attesa/invio del bit di Acknowledge (ACK0), da parte dello slave; 

5\. Invio/ricezione del byte dei dati (DATA), da parte dello slave; 

6\. Attesa/invio del bit di Acknowledge (ACK);

7\. Invio del bit di STOP (P) da parte del master;



## <a name="_toc1361931765"></a><a name="_toc140742980"></a>Libreria SmBus :

Per accedere al bus I2C su Raspberry Pi Zero W, è necessario installare il modulo Python seguente:

- **sudo apt-get install python-smbus** 

Successivamente importare il modulo smbus:

- **import smbus**

Creare un oggetto della classe SMBus per accedere alla funzione Python basata su I2C:

- **<Object name> = smbus.SMBus(I2C port n.)**

È necessario definire la porta I2C utilizzata che può essere la 1 o la 0.

- Esempio: **Bus = smbus.SMBus(1)**

Ora è possibile accedere alla classe **SMBus** mediante il **Bus** object.



<table><tr><th valign="top"><b>Funzione</b>  </th><th valign="top"><b>Tipo</b></th><th valign="top"><b>Return</b></th><th valign="top"><b>Parametri</b></th><th colspan="2" valign="top"><b>Descrizione</b></th></tr>
<tr><td rowspan="3" valign="top"><b>Bus.write_byte_data()</b></td><td rowspan="3" valign="top"><p>M </p><p>S</p></td><td rowspan="3" valign="top">()</td><td valign="top">Device Address</td><td valign="top">Indirizzo del dispositivo a 7 o 10 bit.</td><td rowspan="3" valign="top">Questa funzione viene utilizzata per scrivere i dati nel registro richiesto.</td></tr>
<tr><td valign="top">Register Address</td><td valign="top">` `Indirizzo del registro a cui dobbiamo scrivere.</td></tr>
<tr><td valign="top">Value</td><td valign="top">per passare il valore che deve essere scritto nel registro.</td></tr>
<tr><td rowspan="3" valign="top"><b>Bus.write_i2c_block_data()</b></td><td rowspan="3" valign="top"><p>M</p><p>S</p></td><td rowspan="3" valign="top">()</td><td valign="top"><p>Device Address,</p><p></p></td><td valign="top">Indirizzo del dispositivo a 7 o 10 bit.</td><td rowspan="3" valign="top">Questa funzione viene utilizzata per scrivere un blocco di 32 byte.</td></tr>
<tr><td valign="top"><p>Register Address,</p><p></p></td><td valign="top">Registrare l'indirizzo a cui dobbiamo scrivere i dati.</td></tr>
<tr><td valign="top"><p>[value1, value2,….]</p><p></p></td><td valign="top">scrive un blocco di byte all'indirizzo richiesto.</td></tr>
<tr><td rowspan="2" valign="top"><b>Bus.read_byte_data()</b></td><td rowspan="2" valign="top"><p>M</p><p>S</p><p></p></td><td rowspan="2" valign="top"><p>()</p><p></p></td><td valign="top">Device Address,</td><td valign="top">Indirizzo del dispositivo a 7 o 10 bit.</td><td rowspan="2" valign="top">Questa funzione viene utilizzata per leggere un byte di dati dal registro richiesto.</td></tr>
<tr><td valign="top"><p>Register Address,</p><p></p></td><td valign="top">Registrare l'indirizzo da cui dobbiamo leggere i dati.</td></tr>
<tr><td rowspan="3" valign="top"><b>Bus.read_i2c_block_data()</b></td><td rowspan="3" valign="top"><p>M</p><p>S</p><p></p></td><td rowspan="3" valign="top"><p>()</p><p></p></td><td valign="top">Device Address</td><td valign="top">Indirizzo del dispositivo a 7 o 10 bit.</td><td rowspan="3" valign="top">Questa funzione è utilizzata per leggere un blocco di 32 byte.</td></tr>
<tr><td valign="top">Register Address</td><td valign="top">Registrare l'indirizzo da cui dobbiamo leggere i dati.</td></tr>
<tr><td valign="top">Block of Bytes</td><td valign="top">leggere il numero di byte dall'indirizzo richiesto.</td></tr>
</table>



###
###
###
###
###
###



# <a name="_toc919927569"></a><a name="_toc140742981"></a>Struttura Software
In questo capitolo verranno tratte le implementazioni software per ogni blocco del sistema.
## <a name="_toc215805216"></a><a name="_toc140742982"></a>Raspberry Pi Pico
Una volta effettuate le connessioni hardware viste nel capitolo precedente, sono stati eseguiti i seguenti passi sulla board Raspberry Pi Pico:

- **Installazione del Porting di Raspeberry Pi Pico su Arduino IDE;**
- **Installazione di FreeRTOS per il coordinamento dei Task;**
- **Installazione delle librerie per la lettura dei sensori sul Bus I2C;**
- **Creazione Del codice;**
### <a name="_toc1664135377"></a><a name="_toc140742983"></a>Porting della piattaforma RP2040 su Arduino 

L’Arduino-Pico è stato installato seguendo i passi indicati di seguito:

- copiare alla voce “Additional Boards Manager URLs” l’URL fornito nel repository

![](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.009.png)

- cercare la Pico all’interno del “Boards Manager” ed installare tramite il pulsante “Install”

![image](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.010.png)


### <a name="_toc1654160852"></a><a name="_toc140742984"></a>Implementazione protocollo I2C su Raspberry Pico
Per la gestione delle due linee di comunicazione su bus I2C viene utilizzata la libreria **Wire.h** di Arduino sulla scheda Raspberry PI Pico.

### <a name="_toc1006116314"></a><a name="_toc140742985"></a>Libreria Wire.h :

Dopo aver importato la libreria **Wire.h** nel proprio script è possibile accedere alle seguenti funzioni.

<table><tr><th colspan="1" valign="top"><b>Funzione</b>   </th><th colspan="1" valign="top"><b>Tipo</b> </th><th colspan="1" valign="top"><b>Return</b> </th><th colspan="1" valign="top"><b>Parametri</b> </th><th colspan="1" valign="top"><b>Descrizione</b> </th></tr>
<tr><td colspan="1" rowspan="2" valign="top"><b>begin()</b> </td><td colspan="1" rowspan="2" valign="top"><p><b>M</b>(Master) </p><p>  </p><p><b>S</b>(Slave) </p></td><td colspan="1" valign="top">void </td><td colspan="1" valign="top">() </td><td colspan="1" valign="top">Inizializza il dispositivo come master (da inserire nel setup()). </td></tr>
<tr><td colspan="1" valign="top">void </td><td colspan="1" valign="top">(address) </td><td colspan="1" valign="top"><p>Inizializza il dispositivo come slave e gli assegna l’indirizzo ‘address’  </p><p>(da inserire nel setup()). </p></td></tr>
</table>









**OPERAZIONI DI LETTURA DEI DATI IN ARRIVO** 



<table><tr><th colspan="1" rowspan="2" valign="top"><b>requestFrom()</b> </th><th colspan="1" rowspan="2" valign="top"><b>M</b> </th><th colspan="1" rowspan="2" valign="top">#byte restituiti dalle slave </th><th colspan="1" valign="top">(address, quantity) </th><th colspan="1" valign="top">Usato dal master per richiedere allo slave di indirizzo ‘address’ tanti byte quanti indicati da ‘quantity’. I byte saranno poi recuperati con le funzioni available() e read(). La sequenza viene terminata con uno Stop. </th></tr>
<tr><td colspan="1" valign="top"><p>(address, quantity, stop) </p><p> </p></td><td colspan="1" valign="top">` `Come il precedente. Se ‘stop’ è TRUE viene inviato un segnale di Stop al termine della sequenza, se è FALSE viene inviato un restart (Sr)</td></tr>
<tr><td colspan="1" valign="top"><b>available()</b> </td><td colspan="1" valign="top">M S </td><td colspan="1" valign="top">#byte disponibili </td><td colspan="1" valign="top">() </td><td colspan="1" valign="top"><p>Restituisce il numero di byte disponibili (ovvero inviati da un altro dispositivo);  </p><p>tali byte dovranno essere recuperati con la funzione <b>read()</b>.  </p><p>Tale funzione può essere usata, da un <b>Master</b>, in tal caso dovrà trovarsi dopo una chiamata a <b>requestFrom()</b>. </p><p>Oppure da uno <b>Slave</b> dovrà essere chiamata all’interno della funzione indicata in <b>onReceive().</b>  </p></td></tr>
<tr><td colspan="1" valign="top"><b>read()</b> </td><td colspan="1" valign="top">M S </td><td colspan="1" valign="top">Il prossimo byte ricevuto </td><td colspan="1" valign="top">() </td><td colspan="1" valign="top"><p>Legge un byte trasmesso da uno slave ad un master dopo una chiamata a <b>requestFrom().</b></p><p>Legge un byte trasmesso da un master ad uno slave. </p></td></tr>
</table>



**OPERAZIONI DI INVIO DI DATI AD ALTRI DISPOSITIVI** 


<table><tr><th colspan="1" valign="top"><b>beginTransmission()</b> </th><th colspan="1" valign="top">M </th><th colspan="1" valign="top">void </th><th colspan="1" valign="top">(address) </th><th colspan="2" valign="top">Inizia una trasmissione I2C verso lo slave di indirizzo ‘address’. Successivamente bisognerà mettere in una coda i byte da trasmettere tramite la funzione write(), ed infine inviare tali byte con la funzione endTransmission(). </th></tr>
<tr><td colspan="1" rowspan="3" valign="top"><b>write()</b> </td><td colspan="1" rowspan="3" valign="top">M S </td><td colspan="1" rowspan="3" valign="top">#byte inviati </td><td colspan="1" valign="top">(value) </td><td colspan="1" valign="top">Invia ‘value’ come singolo byte. </td><td colspan="1" rowspan="3" valign="top">Tale funzione può essere usata sia da un master che da uno slave. Nel primo caso dovrà essere preceduta da un beginTransmission() e seguita da un endTransmission(), nel secondo caso no. </td></tr>
<tr><td colspan="1" valign="top">(string) </td><td colspan="1" valign="top">Invia la stringa ‘string’ come serie di singoli byte </td></tr>
<tr><td colspan="1" valign="top">(data, length) </td><td colspan="1" valign="top">Invia un array di dati come singoli byte; la lunghezza dell’array è ‘length’</td></tr>
<tr><td colspan="1" rowspan="2" valign="top"><b>endTransmission()</b> </td><td colspan="1" rowspan="2" valign="top">M </td><td colspan="1" rowspan="2" valign="top">Byte che indica lo stato della trasmissione </td><td colspan="1" valign="top">() </td><td colspan="2" valign="top">Conclude la trasmissione ad uno slave iniziata con beginTransmission() e write() inviando i dati ed uno Stop. </td></tr>
<tr><td colspan="1" valign="top">(stop) </td><td colspan="2" valign="top">Dopo aver inviato i dati, invia uno STOP se ‘stop’ è TRUE; se ‘stop’ è FALSE invia un segnale di Restart (Sr). </td></tr>
</table>

**GESTIONE DELLE OPERAZIONI DELLO SLAVE IN RISPOSTA ALLE INDICAZIONI DEL MASTER** 



|**onRequest**() |S |void |(handler) |Serve per indicare quale funzione (handler) deve essere chiamata nel momento in cui un master richiede dati a questo slave; la funzione che deve essere chiamata non deve avere parametri né deve restituire alcunché. |
| :- | :- | :- | :- | :- |
|**onReceive**() |S |void |(handler) |Serve per indicare quale funzione (handler) deve essere chiamata nel momento in cui questo slave riceve dati da un master; la funzione che deve essere chiamata non deve restituire alcunché e deve avere come unico parametro il numero di byte ricevuti dal master. |

### <a name="_toc1271163580"></a><a name="_toc140742986"></a>FreeRTOS

**FreeRTOS** è un sistema operativo open-source progettato specificamente per sistemi embedded e applicazioni in tempo reale che fornisce **multitasking** efficiente e capacità di **scheduling** per microcontrollori e microprocessori.

**Caratteristiche**

- **Gestione dei task**
  - Consente di creare più task, ognuno con la propria priorità, stack e contesto di esecuzione. I task possono essere preemptive o cooperativi, a seconda della configurazione, consentendo un multitasking efficiente.

- **Scheduling**
  - Utilizza uno **scheduler** **preemptive** basato sulla priorità, che garantisce l'esecuzione dei task ad alta priorità prima di quelli a bassa priorità. Supporta algoritmi di scheduling a priorità fissa e a priorità dinamica.

- **Gestione degli interrupt** 
  - Fornisce meccanismi sicuri per la gestione degli interrupt. Consente di creare interrupt service routine (ISR) e di comunicare tra ISR e task utilizzando primitive di sincronizzazione leggere come semafori, code e flag di evento.


- **Sincronizzazione e comunicazione**
  - Offre vari meccanismi di sincronizzazione per la comunicazione e il coordinamento tra i task. Questi includono semafori, mutex, flag di evento, code e software timer. Questi meccanismi consentono ai task di sincronizzare le loro azioni, condividere risorse e comunicare tra di loro in modo efficiente.

- **Gestione della memoria**
  - Fornisce uno schema di gestione della memoria heap semplice ed efficiente per l'allocazione dinamica della memoria. Offre funzioni di allocazione della memoria come pvPortMalloc() e vPortFree(), consentendo ai task di allocare e liberare la memoria dinamicamente.

- **Modalità idle a basso consumo energetico**
  - Supporta una modalità idle a basso consumo energetico, che consente al sistema di entrare in uno stato di basso consumo durante i periodi di inattività. Questa funzione aiuta a risparmiare energia nei sistemi alimentati a batteria o a basso consumo energetico.












Codice

Il codice ha la seguente struttura:

![Immagine che contiene testo, Carattere, schermata

Descrizione generata automaticamente](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.011.jpeg)

Troviamo il file principale **Pico\_SOD.ino** affiancato dall’**header** **variables\_definition.hpp** e la cartella **sensors**, all’interno della quale troviamo gli **headers** relativi ai sensori.

In particolare:

- **Pico\_SOD.ino**
  - È il codice principale in cui vengono richiamati tutti gli headers (variables.. e sensori) ed al suo interno troviamo i vari task FreeRTOS più altre funzioni per la gestione in slave dell’I2C0.


- **Variables\_definition.hpp**
  - È **l’header** che accompagna il **.ino** per alleggerire il codice da tutte le definizioni e variabili globali in modo da rendere il codice più leggibile.

- **Cartella sensors**
  - In questa cartella troviamo tutti gli headers dei rispettivi sensori con all’interno le varie funzioni di setup() e lettura dei dati.












Librerie Sensori 

Per i sensori sono state usate le seguenti librerie:

- **BMP280**
  - **GitHub**: <https://github.com/adafruit/Adafruit_BMP280_Library>

Inizializzando il sensore come **Adafruit\_BMP280 bmp(&Wire…);**

la libreria consente di leggere i dati del sensore richiamando le rispettive funzioni per “sottosensore”:

- **Temperatura**
  - bmp.readTemperature()
- **Pressione**
  - bmp.readPressure()
- **Altitudine**
  - bmp.readAltitude()

- **BH1750** 
  - **GitHub**: <https://github.com/wollewald/BH1750_WE>

Inizializzando il BH1750 come **BH1750\_WE myBH1750 = …;** 

la libreria consente di leggere i dati del sensore richiamando la funzione:

- **Luminosità**
  - myBH1750.getLux()

- **DS1307** 
  - **GitHub**: <https://reference.arduino.cc/reference/en/libraries/rtclib>

La libreria consente di gestire i dati dell’RTC tramite alcune funzioni:

Inizializzando l’RTC come RTC**\_DS1307 rtc;**

- **Lettura ora e data dell’RTC**
  - DateTime now = rtc.now();
- **Scrittura ora e data sull’RTC**
  - rtc.adjust(dateTime);









Utilizzo delle librerie nel progetto 

Per quanto riguarda le librerie relative ai sensori citati poc’anzi:

- **BMP280**
  - Il sensore viene inizializzato col nome di **bmp** ed impostato per lavorare sul **bus I2C1 (&Wire1)**, il setup resta quello riportato sulla libreria:

![Immagine che contiene testo, schermata, Carattere

Descrizione generata automaticamente](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.012.png)

- Per quanto riguarda la lettura dei dati, sono state definite tre funzioni:

![Immagine che contiene testo, schermata, Carattere

Descrizione generata automaticamente](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.013.png)

- Sia il **setup**, sia le tre **funzioni di lettura** vengono poi richiamate dal task **FreeRTOS** relativo al **BMP280**.









- **BH1750**
  - Il BH1750 viene anch’esso inizializzato sul bus **I2C1:**

![Immagine che contiene testo, schermata, Carattere

Descrizione generata automaticamente](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.014.png)

- Il setup resta quello fornito dalla libraria
- Viene aggiunta la funzione di lettura del dato, **BH1750\_data\_read()** che sarà poi richiamata dal task **FreeRTOS** relativo al **BH1750**.

- **DS1307**
  - Il setup resta quello fornito dalla libreria:

![Immagine che contiene testo, schermata, Carattere

Descrizione generata automaticamente](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.015.png)







- Viene aggiunta la funzione di lettura del **Timestamp:**

![Immagine che contiene testo, Carattere, schermata, numero

Descrizione generata automaticamente](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.016.png)

- **FreeRTOS**
  - Ulteriore libreria e cuore del progetto è **FreeRTOS** introdotto precedentemente.

Come spiegato, viene inclusa come una comunissima **libreria** e consente di creare **task** che possono essere eseguiti attraverso uno **scheduler**.

- **Nel progetto**
  - Nel nostro caso, vengono utilizzati quattro task di FreeRTOS, in particolare uno per ciascun sensore (ossia tre) più un ulteriore task, il Task\_monitor.
    - **BMP280\_Task** 
      - Setup eseguito e ripetuto finché il sensore non viene inizializzato correttamente.
      - Loop della lettura dei dati e salvataggio degli stessi su variabili globali.

- **BH1750\_Task** 
  - Setup eseguito e ripetuto finché il sensore non viene inizializzato correttamente.
  - Loop della lettura dei dati e salvataggio degli stessi su variabili globali.

- **RTC\_Task** 
  - Setup eseguito e ripetuto finché il sensore non viene inizializzato correttamente.
  - Loop della lettura del timestamp UNIX e salvataggio su variabile globale.

- **Task\_monitor**
  - Si occupa semplicemente di riportare su seriale, qualora il **LOG** venga abilitato da codice, i dati attuali di tutti i sensori.





### <a name="_toc140742987"></a>FreeRTOS nel nostro progetto

Di seguito vengono riportati i codici dei singoli task FreeRTOS:

- **Codice Task BMP280**

![Immagine che contiene testo, schermata

Descrizione generata automaticamente](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.017.png)

- **Codice Task BH1750**

![Immagine che contiene testo, schermata, software

Descrizione generata automaticamente](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.018.png)



- **Codice Task RTC**

![](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.019.png)

- **Parte del codice del Task Monitor**

![Immagine che contiene testo, schermata, software, Carattere

Descrizione generata automaticamente](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.020.png)


Per motivi di testing è stata aggiunta la possibilità di abilitare o meno il **semaforo** per la gestione del bus **I2C1.**

![](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.021.png)

- **I2C Slave**

L’obiettivo finale della Pico è quello di far giungere, sulla base delle richieste, i dati dei sensori alla Zero attraverso l’I2C, in particolare il bus I2C0 sul quale la Pico viene impostata in modalità Slave nei confronti della Zero (Master).

Per poter riceve e di conseguenza elaborare le richieste, sono presenti le funzioni OnReceive() e OnRequest().

Nel codice possiamo trovare le due funzioni così definite:

- **OnReceive()**

![Immagine che contiene testo, schermata, software, schermo

Descrizione generata automaticamente](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.022.png)

Nella **OnReceive()**,** la Pico è programmata per rispondere in base ai registri richiesti:

- **BMP\_TEMP\_REG**
  - Risponde al registro **0x41**
  - La richiesta viene gestita dall’OnRequest()
- **BMP\_PRESS\_REG**
  - Risponde al registro **0x42**
  - La richiesta viene gestita dall’OnRequest()
- **BMP\_ALT\_REG**
  - Risponde al registro **0x43**
  - La richiesta viene gestita dall’OnRequest()
- **BH1750\_LUX\_REG**
  - Risponde al registro **0x44**
  - La richiesta viene gestita dall’OnRequest()


- **RTC\_REG**
  - Risponde al registro **0x45**
  - La richiesta viene gestita dall’OnRequest()

- **SYNC\_TIME**
  - Risponde al registro **0x46**
  - Il **SYNC\_TIME** ha una gestione diversa poiché è la Pico che in questo caso deve ricevere i dati (timestamp proveniente dalla Zero).

Al **SYNC\_TIME** succedono ben 4Byte con al loro interno il timestamp UNIX, quindi per poter gestire questa richiesta, alla ricezione del Byte 0x46 viene abilitato il flag **SYNC\_TS** che da questo momento disabiliterà la gestione dei registri ricevuti, quindi il successivo byte ricevuto viene inserito in una stringa, anche se questo dovesse combaciare con uno dei suddetti registri. Stessa cosa accade dal terzo al quinto Byte.

Dopodiché la stringa viene convertita in un array di Byte, ottenendo così il timestamp che verrà utilizzato per sincronizzare l’**RTC**.

Una volta fatto ciò, viene disabilitato il flag **SYNC\_TS** (il successivo Byte ricevuto sarà un registro) per permettere di gestire altri dati dei sensori richiesti.
















- **OnRequest()**

![Immagine che contiene testo, schermata, software

Descrizione generata automaticamente](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.023.png)

Nel **OnRequest()** viene letto il registro che l’**OnReceive()** ha ricevuto e** salvato nella variabile globale **Received\_Command** e viene poi gestita la richiesta, andando a scrivere sull’I2C0 il dato del sensore richiesto.



## <a name="_toc2077330746"></a><a name="_toc140742988"></a>Raspberry Pi Zero W
### <a name="_toc1287755876"></a><a name="_toc140742989"></a>Sistema operativo utilizzato: 
#### <a name="_toc140742990"></a>*Diet-Pi*

![Immagine che contiene schermata, verde, cartone animato, Elementi grafici

Descrizione generata automaticamente](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.024.png)

**DietPi** è una distribuzione leggera e ottimizzata del sistema operativo Linux, basata su **Debian**, progettata principalmente per l'uso su dispositivi a bassa potenza, come Raspberry Pi per l’appunto. L'obiettivo principale di DietPi è fornire un'esperienza di sistema operativo minimale, efficiente ed estremamente snella, ottimizzata per sfruttare al massimo le risorse hardware limitate dei dispositivi embedded.
##### Caratteristiche

- **Leggerezza**
  - È noto per essere estremamente leggero e richiedere poca memoria RAM e risorse di archiviazione. Questo lo rende ideale per dispositivi con specifiche hardware limitate, come le SBC.

- **Configurazione semplificata**
  - Offre un'interfaccia di configurazione utente facile da usare, accessibile tramite un terminale interattivo o un'interfaccia web. Questo consente agli utenti di personalizzare facilmente il sistema operativo in base alle proprie esigenze e preferenze.

- **Selezione di software**	
  - Offre una vasta gamma di software preconfigurato e ottimizzato per le diverse esigenze degli utenti. Puoi scegliere tra vari pacchetti di software per server, desktop, media center, reti, sicurezza e altro ancora durante l'installazione.


- **Aggiornamenti e manutenzione**
  - Semplifica il processo di aggiornamento del sistema operativo e dei software installati, fornendo un sistema di gestione del software semplice da usare.



- **Comunità attiva**
  - Gode di una comunità attiva di sviluppatori e utenti che contribuiscono con suggerimenti, correzioni di bug e nuove funzionalità, rendendo il sistema operativo sempre più stabile e ricco di funzionalità.

### <a name="_toc140742991"></a>Implementazione nel progetto

Il software che gestisce i compiti svolti dalla Raspberry Pi Zero W è stato scritto in Python e si occupa di fornire le richieste dei dati, gestendo le richieste ricevute via MQTT da parte dell’applicazione web, pubblicati con *topic* ***WEB\_REQ***, e rispondendo con i singoli sensori richiesti, pubblicando i rispettivi dati, ognuno col proprio *topic* (***BMP280***, ***BH1750*** e ***RTC***). Il tutto è gestito attraverso lo script **main\_mqtt.py**.

Il software ha la seguente struttura:

![Immagine che contiene testo, schermata, Carattere, software

Descrizione generata automaticamente](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.025.png)

Troviamo il suddetto script **main\_mqtt.py** accostato dalla cartella **pico\_sensors** che al suo interno presenta l’omonimo script **pico\_sensor.py** e la cartella **libraries**, che a sua volta, ha al suo interno le 3 librerie (**BMP280.py**, **BH1750.py** e **RTC.py**). Inoltre, è stato creato il file **main\_mqtt.service** che permette allo script **main\_mqtt.py** di essere avviato automaticamente al boot del sistema.

- **main\_mqtt.py**
  - è lo script principale che viene avviato al boot del sistema, è composto da un thread sempre in esecuzione che gestisce la comunicazione MQTT, comportandosi sia da **Subscriber** (ricevendo richieste) che da **Publisher** (pubblicando i dati dei sensori richiesti). In esso viene importato anche **pico\_sensor.py,** che permette la comunicazione via I2C con la **Pico,** che a sua volta importa le librerie **BMP280.py**, **BH1750.py** e **RTC.py.**
- **BMP280.py**, **BH1750.py** e **RTC.py**
  - non sono librerie che controllano direttamente i sensori, come si potrebbe erroneamente pensare, ma permettono di gestire le richieste ai singoli sensori mediante la **Pico** collegata in **I2C**.


### <a name="_toc140742992"></a>Codice
Di seguito vengono riportate parti del codice Python sviluppato:

Librerie utilizzate:

- **Paho MQTT**
  - È una libreria open-source sviluppata da **Eclipse Paho** che fornisce implementazioni del protocollo **MQTT** (Message Queue Telemetry Transport) per diverse piattaforme di programmazione. **MQTT** è un protocollo di messaggistica leggero e di tipo **publish-subscribe** utilizzato per la comunicazione tra dispositivi con restrizioni di risorse, come sensori e dispositivi IoT (Internet of Things).
- **JSON**
  - è inclusa di serie in Python e offre il supporto per la codifica e decodifica di dati nel formato JSON. Il JSON è un formato di dati molto comune e utilizzato per lo scambio di informazioni tra sistemi diversi. La libreria json consente a Python di lavorare con dati JSON in modo semplice ed efficiente.




















Di seguito, è riportato il codice del file **main\_mqtt.py**.

![](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.026.png)

Nel codice, escludendo gli import e le variabili globali, troviamo:

- **connect\_web\_request()** 
  - Crea e restituisce un client MQTT chiamato che è utilizzato per gestire le richieste di dati provenienti da un'applicazione esterna. Viene definito **on\_connect(),** un gestore di connessione che verrà chiamato quando il client si connette al broker MQTT. La funzione si connette al broker specificando l'indirizzo e la porta del broker e restituisce il client configurato.

- **subscribe\_WEB\_REQUEST()**
  - Gestisce la sottoscrizione al topic **WEB\_REQ\_TOPIC**, in modo da ricevere le richieste di dati dai sensori. Viene definito un gestore di messaggi **on\_message()** che verrà chiamato quando il client riceve un messaggio sul topic sottoscritto. Quando viene ricevuta una richiesta, la funzione **on\_message\_command()** verrà chiamata per gestire la richiesta in base al sensore specificato nel messaggio.

- **on\_message\_command(data)**
  - Questa funzione viene chiamata quando il client **client\_web\_request** riceve un messaggio sul topic **WEB\_REQ\_TOPIC**. Analizza il messaggio JSON ricevuto per determinare quale sensore è richiesto e invia una richiesta al sensore appropriato utilizzando la libreria **pico\_sensors**. I dati ricevuti dai sensori vengono quindi pubblicati sul broker MQTT utilizzando i topic appropriati (**BMP280\_TOPIC**, **BH1750\_TOPIC**, **RTC\_TOPIC**).

- **run()**
  - Funzione della libreria MQTT che avvia tutto il processo di gestione del client.






















<a name="_toc1622904011"></a><a name="_toc140742993"></a>Di seguito, è riportato il codice del file **main\_mqtt.py**

![Immagine che contiene testo, elettronica, schermata, computer

Descrizione generata automaticamente](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.027.png)

Nel codice troviamo:

- **Gestione I2C**
  - Nella prima parte del codice troviamo l’inizializzazione della comunicazione I2C mediante la libreria **SMBUS.**

- **BMP280\_data\_read(), BH1750\_data\_read() e RTC\_data\_read()**
  - Queste tre funzioni hanno tutte la stessa struttura e si occupano di restituire i valori in formato JSON relativo al sensore, generato andando a richiamare la funzione **generate\_json\_data(RTC\_sensor)**, fornita dalle librerie **BMP280.py, BH1750.py e RTC.py.**

- **Sync\_time\_pico()**
  - Si occupa della sincronizzazione dell’**RTC** collegata alla **Pico**, oltre a poter essere richiamata manualmente, viene eseguita all’avvio dello script / sistema per sincronizzare immediatamente l’**RTC** con l’ora e la data attuale.






































Di seguito viene riportato il codice della libreria **BMP280.py** che permette l’interazione con l’omonimo sensore mediante la **Pico** tramite l’**I2C0:**

![Immagine che contiene testo, schermata, software, Sistema operativo

Descrizione generata automaticamente](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.028.png)**

Tra le funzioni all’interno della classe **bmp\_sensor** troviamo:

- **\_write\_byte()**
  - Funzione privata che richiama la funzione di libreria per scrivere su I2C0.
- **\_write\_i2c\_block\_data()**
  - Funzione privata che richiama la funzione di libreria per scrivere un blocco di dati su I2C0.
- **\_read\_byte()**
  - Funzione privata che richiama la funzione di libreria per leggere su I2C0.
- **\_data\_exchange()**
  - Funzione privata che si occupa della richiesta del sensore (scritture del registro relativo sull’I2C0 mediante \_**\_write\_byte()**) e della successiva lettura del dato corrispondente ricevuto in risposta dalla Pico (funzione **\_read\_byte()).**
- **read\_temperature()**
  - Funzione che passa a **\_data\_exchange()** il registro relativo alla **temperatura** che si occuperà di restiturne il dato.
- **read\_pressure()**
  - Funzione che passa a **\_data\_exchange()** il registro relativo alla **pressione** che si occuperà di restiturne il dato.
- **read\_altitude()**
  - Funzione che passa a **\_data\_exchange()** il registro relativo all’**altitudine** che si occuperà di restiturne il dato.
- **generate\_json\_data()**
  - Funzione che genera e restituisce il json relativo al sensore (nel caso del **BMP280** ne troviamo tre), includendone il timestamp restituito dall’**RTC**.















Di seguito viene riportato il codice della libreria **BH1750.py** che permette l’interazione con l’omonimo sensore mediante la **Pico** tramite l’**I2C0:**

![Immagine che contiene testo, schermata, software

Descrizione generata automaticamente](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.029.png)


Tra le funzioni all’interno della classe **bh1750\_sensor**, di cui omettiamo quelle già presentate nel **BMP280.py**, troviamo:

- **read\_light\_intensity()**
  - Funzione che passa a **\_data\_exchange()** il registro relativo all’illuminamento che si occuperà di restiturne il dato.

- **generate\_json\_data()**
  - Funzione che genera e restituisce il json relativo al sensore, includendone il timestamp restituito dall’**RTC**.
























Di seguito viene riportato il codice dell’**RTC.py:**

![Immagine che contiene testo, schermo, schermata, computer

Descrizione generata automaticamente](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.030.png)

Anche qui tra le funzioni all’interno della classe **timestamp\_sensor** omettiamo quelle già presentate precedentemente:

- **\_read\_timestamp()**
  - Funzione privata che richiama la funzione di libreria per leggere su I2C0, poiché il **timestamp** è scritto su quattro byte, rispetto agli altri sensori, viene utilizzata la funzione di libreria **read\_i2c\_block\_data()**.

- **\_convert\_byte\_to\_human\_ts()**
  - Funzione privata che si occupa di convertire il **timestamp** in formato **Unix** in una stringa comprensibile.

- **sync\_datetime()**
  - Funzione che legge il **timestamp** di sistema (**Diet-Pi**) e lo invia sull’**I2C0.**

- **read\_timestamp()**
  - Funzione che legge il **timestamp** ricevuto dalla **Pico** tramite l’**I2C0.**
- **generate\_human\_ts()**
  - Funzione pubblica che restituisce il timestamp già convertito in un formato leggibile tramite l’ausilio del **\_convert\_byte\_to\_human\_ts().**

- **generate\_json\_data()**
  - Funzione che genera e restituisce il json relativo al timestamp.
















## Applicazione web full stack
Il compito principale dell’applicazione web è quello di visualizzare i dati in tempo reale dei sensori provenienti dal broker MQTT pubblicati nei vari topic, visualizzare i dati storici mediante grafici a serie temporali e richiedere la sincronizzazione del modulo RTC tramite l’invio di un messaggio apposito sul topic “RTC\_SYNC”. Infine, sono presenti dei bottoni che permettono di richiedere in tempo reale, i dati dei sensori memorizzandoli nel database oltre che ad un ulteriore bottone per monitorare il timestamp impostato nel modulo RTC.

Come già accennato, l’applicazione web full stack è composta da tre elementi essenziali:

- **MongoDB**: database NOSQL che contiene i dati storici dei sensori;
- **Node JS**: web server che gestisce la connessione al database MongoDB e le operazioni di memorizzazione ed estrazione dei dati;
- **React JS**: framework che realizza la pagina web che visualizza i dati di interesse;
##
### <a name="_toc2142575988"></a><a name="_toc140742994"></a>Mongo DB
Il database è composto da due **collections**:

- **bh1750:** è una tabella composta da due campi (**timestamp** e **lux**). Il primo campo memorizza il timestamp fornito dall’RTC al momento della richiesta del dato e lux fornisce il valore di luminosità fornito dal BH1750;
- **bmp280:** è una tabella composta da quattro campi (**timestamp**, **temperature**, **pressure** e **altitude**). Il primo campo memorizza il timestamp fornito dall’RTC al momento della richiesta del dato, gli altri forniscono rispettivamente la temperatura, pressione e altitudine forniti dal BMP280.
### <a name="_toc2009247700"></a><a name="_toc140742995"></a>Backend

Il server NodeJS è riassumibile in tre funzioni principali:

1. **Connessione con il database**: all’avvio del server tramite nodemon, il server effettua una connessione al database MongoDB remoto;
1. **Estrazione dei dati dalle collections**: ci sono due rotte dedicate (*getBMPData e getBHData*) che estraggono tutti i documenti della collection in formato JSON;
1. **Memorizzazione dei nuovi dati**: ci sono due rotte dedicate (*storeBHData* e *storeBMPData*) che memorizzano i nuovi dati all’interno delle collections per avere lo storico dei dati.


### <a name="_toc1798883428"></a><a name="_toc140742996"></a>Frontend

![](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.031.png)![](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.032.png)![](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.033.png)

Sopra è riportata la schermata principale del **sito web**. Come si può vedere, è suddiviso in **due aree principali**: la prima parte è dedicata **ai dati in tempo reale**, la seconda alla **visualizzazione dei dati storici**. Quando sui topic MQTT “BPM280” e “BH1750” vengono pubblicati dei messaggi, i valori dei singoli campi vengono mostrati nella pagina e successivamente essi vengono **memorizzati** nel database MongoDB ed infine viene **aggiornato** ogni singolo grafico corrispondente al nuovo dato.
Qui sotto, viene mostrata la visualizzazione dei dati in presenza di un messaggio proveniente dal topic “BMP280”.

![Immagine che contiene testo, schermata, linea, Carattere

Descrizione generata automaticamente](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.034.png)

Qui sotto, viene mostrata la visualizzazione dei dati in presenza di un nuovo messaggio proveniente dal topic “BH1750” sopraggiunto dopo il messaggio precedente. 

![Immagine che contiene testo, schermata, linea, diagramma

Descrizione generata automaticamente](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.035.png)

Se viene premuto il bottone “**aggiorna**”, ReactJS invia una richiesta di un nuovo dato via MQTT, al topic “**WEB\_REQ**” con il messaggio:

**{**

`     `**“sensor”: “BMP280”**

**}**

oppure:

**{**

`    `**“sensor”: “BH1750”**

**}**

in funzione del bottone premuto.

A seguito della richiesta sul topic “**WEB\_REQ**”, la Raspberry PI ZERO invia sul topic “**BMP280**” i nuovi dati che vengono poi gestiti come già spiegato in precedenza.

Se viene premuto il bottone “**Sync**”, viene inviato un messaggioo sul topic “WEB\_REQ”:

**{**

`    `**“sensor”: “RTC\_SYNC”**

**}**

che ricevuto dalla Raspberry PI ZERO, va ad impostare il timestamp dell’RTC alla data corrente, restituendo ad operazione conclusa, un messaggio sul topic “RTC” così formato:

***RTC sync successufull!***

La pagina web mostra il messaggio per **5 secondi** in questo modo:

![Immagine che contiene testo, schermata, linea, software

Descrizione generata automaticamente](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.036.png)

Se viene premuto il bottone “**Leggi**”, viene mostrato a schermo il **timestamp attuale** impostato sull’RTC in questo modo:

![Immagine che contiene testo, schermata, linea, numero

Descrizione generata automaticamente](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.037.png)

Successivamente, viene mostrato il valore del timestamp dell’RTC **in tempo reale**, aggiungendo un secondo al timestamp iniziale per mantenere il valore aggiornato come mostrato di seguito:

![Immagine che contiene testo, schermata, Carattere, numero

Descrizione generata automaticamente](Immagini/Aspose.Words.2c704c32-f6aa-4507-a45b-d57decfd28a3.038.png)
2

