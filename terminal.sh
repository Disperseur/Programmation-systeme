#!/bin/bash

CMD="cd Prog_sys/Projet/projet" #aller dans le répertoire, à personnaliser

CMD2="./client localhost 2000"	#exécuter la commande client

for i in {1..4}	#répéter 4 fois
do

  gnome-terminal -- bash -c "$CMD; $CMD2; exec bash" &

done

