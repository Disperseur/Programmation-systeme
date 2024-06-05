# Readme du projet PSE: *Tournoi de Morpion*

Projet: serveur de matchs de morpion

## Utilisation

* Lancer le serveur dans un premier terminal via:
````$./Serveur 2000````
* Ouvrir quatres autres terminaux et lancer quatre clients via:
````$./client localhost 2000````.
Cela permet de simuler quatre clients en local sur une seule machine.

## Fonctionnement
Le jeu fonctionne en tour par tour. Deux matchs se déroulent simultanément (il est possible d'en mettre plus évidemment). Lorsque c'est son tour de jouer, le joueur est invité à saisir les coordonées de l'emplacement ou il souhaite placer sa marque au format "x y". La grille est mise à jour automatiquement et le client attends que le joueur adverse joue son coup pour continuer.



## Choix techniques

* Etant donné la communication avec des chaines de caractères déjà présente dans le TP8, le choix à été fait de la conserver. Une communication dans l'autre sens pour le serveur vers un joueur à été ajoutée. Une trame de message standard à été choisie pour uniformiser les messages échangés entre le serveur et les clients. Elle prends la forme suivante:
``"%c %d %d\n"`` (description via les formatteurs de printf).
La partie est gérée par le serveur.


Chaque worker s'occupe d'un client. Il récupère le coup joué (dont la validité est vérifiée par le client lui même) et modifie la grille en conséquence. La grille de chaque match existe en trois exemplaires: un dans chaque client d'une partie et un dans la structure qui décrit un match en cours dans le serveur.




Pour le débug, le serveur reçoit les coups joués par chaque client sous la forme "c <pos_x> <pos_y>"