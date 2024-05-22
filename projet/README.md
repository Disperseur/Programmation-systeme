# Readme du projet PSE

Projet: tournoi de morpion


## Choix techniques

* Etant donné la communication avec des chaines de caractères déjà présente dans le TP8, on peut la garder pour que les clients (joueurs) ne transmettent que leurs coups. Il faut ajouter une communication dans l'autre sens pour le serveur vers un joueur. La partie est gérée par le serveur


Chaque worker s'occupe d'un client. Il s'assure que le coup demandé est possible et modifie la grille en conséquence. Il déclenche une vérification de la grille par le main. La grille de chaque match est dans une structure match contenue dans un tableau des matchs global.

## Implémentations

* Communication à double sens entre client et serveur
* attributs match et joueur aux workers
* attente de la connexion de tout les joueurs avant le démarrage des matchs


## Trucs à ajouter (?)

- gestion d'un match  