# Readme du projet PSE: *Serveur de Morpion*

Projet: serveur de matchs de morpion

## Contexte

Dans le contexte d'une LAN de Morpion, les matchs doivent commencer en même temps et tous se terminer avant de passer aux matchs suivants. C'est la raison pour laquelle le serveur attends que tout les clients soient connectés pour commencer les matchs, et qu'il attend la fin de tout les matchs pour s'arrêter.

## Compilation

Pour compiler la bibliothèque, exécuter dans le dossier "modules" la commande:
$./make
Puis pour compiler le projet, dans le dossier "src" exécuter la commande:
$./make


## Utilisation

* Lancer le serveur dans un premier terminal via:
````$./Serveur 2000````

Choisir le nombre de matchs que l'on autorise simultanément.

* Ouvrir deux fois plus de terminaux qu'il y a de matchs (un terminal par joueur) et lancer autant de clients via:
````$./client localhost 2000````.
Cela permet de simuler les clients en local sur une seule machine.

## Fonctionnement
Le jeu fonctionne en tour par tour. Les matchs se déroulent simultanément obligatoirement. C'est à dire que si il y a deux matchs, les quatre joueurs doivent obligatoirement tous se connecter pour que les matchs commencent. Lorsque c'est son tour de jouer, le joueur est invité à saisir les coordonées de l'emplacement ou il souhaite placer sa marque au format "x y". La grille est mise à jour automatiquement et le client attends que le joueur adverse joue son coup pour continuer.



## Choix techniques

* Etant donné la communication avec des chaines de caractères déjà présente dans le TP8, le choix à été fait de la conserver. Une communication dans l'autre sens pour le serveur vers un joueur à été ajoutée. Une trame de message standard à été choisie pour uniformiser les messages échangés entre le serveur et les clients. Elle prends la forme suivante:
``"%c %d %d\n"`` (description via les formateurs de printf).
La partie est gérée par le serveur.


Chaque worker s'occupe d'un client. Il récupère le coup joué (dont la validité est vérifiée par le client lui même) et modifie la grille en conséquence. La grille de chaque match existe en trois exemplaires: un dans chaque client d'une partie et un dans la structure qui décrit un match en cours dans le serveur.




Pour le débug, le serveur reçoit les coups joués par chaque client sous la forme "c <pos_x> <pos_y>"