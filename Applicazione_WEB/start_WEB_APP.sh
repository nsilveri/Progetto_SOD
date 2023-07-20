#!/bin/bash

# Termina un processo dato il suo nome utilizzando il segnale SIGINT
termina_processo() {
  processo=$1
  if pgrep -x "$processo" > /dev/null; then
    echo "Terminazione del processo $processo..."
    pkill -SIGINT "$processo"
    sleep 1
    echo "Il processo $processo è stato terminato."
  fi
}

# Termina i processi "nodemon" e "npm" se sono già in esecuzione
termina_processo "nodemon"
termina_processo "npm"

# Esegui nodemon in background nella cartella backend
cd /home/dietpi/Progetto_SOD/backend
nodemon &

# Aggiungi un ritardo di 1 secondo
sleep 1

# Esegui npm start in background nella cartella frontend
cd /home/dietpi/Progetto_SOD/frontend
npm start &
