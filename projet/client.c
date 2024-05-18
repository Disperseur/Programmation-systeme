/*
Fonctionnalités du client du jeu Morpion
- afficher la grille (grille qui est gérée par le serveur donc il faut un dialogue..)
- acquérir le choix du joueur quand c'est son tour (système de flag pour bloquer lors du tour de l'autre joueur)
- afficher victoire/défaite/match nul puis quitter
*/



#include "pse.h"
#include "morpion.h"
#define CMD "client"

#define DEBUT_MATCH 's'
#define TOUR_JOUEUR 't'
#define FIN_PARTIE  'f'


int main(int argc, char *argv[]) {
     // initialisation du client
    int sock, ret;
    struct sockaddr_in *adrServ;
    char ligne_envoyee[LIGNE_MAX];
    char ligne_recue[LIGNE_MAX];
    int lgEcr;

    if (argc != 3)
        erreur("usage: %s machine port\n", argv[0]);

    // printf("%s: creating a socket\n", CMD);
    sock = socket (AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        erreur_IO("socket");

    // printf("%s: DNS resolving for %s, port %s\n", CMD, argv[1], argv[2]);
    adrServ = resolv(argv[1], argv[2]);
    if (adrServ == NULL)
        erreur("adresse %s port %s inconnus\n", argv[1], argv[2]);

    // printf("%s: adr %s, port %hu\n", CMD,
    //         stringIP(ntohl(adrServ->sin_addr.s_addr)),
    //         ntohs(adrServ->sin_port));

    // printf("%s: connecting the socket\n", CMD);
    ret = connect(sock, (struct sockaddr *)adrServ, sizeof(struct sockaddr_in));
    if (ret < 0)
    erreur_IO("connect");



    // boucle principale
    /*
    Algorithme général:
        tant que personne ne gagne:
            on attends notre tour
            on donne les coordonnees du coup a jouer
            on teste si les coordonnees sont ok
            on MaJ la grille et on envoie le coup au serveur
            on affiche la grille MaJ
        on affiche le resultat
        on met en attente pour le match suivant (ou on deconnecte tout simplement)
    */

    system("clear");
    // on affiche le numero de match et l'equipe du joueur
    lireLigne(sock, ligne_recue);
    printf("%s\n", ligne_recue);

    // initialisation de la grille de jeu
    initialiserGrille();
    // affichage de la grille
    afficherGrille();

    

    // while(strcmp(ligne_recue, "fin\n") != 0) {
    while(ligne_recue[0] != FIN_PARTIE) {
        lireLigne(sock, ligne_recue);

        if (ligne_recue[0] == DEBUT_MATCH) {
            printf("Debut de la partie !\n");
        }

        if (ligne_recue[0] == TOUR_JOUEUR) {
            // a notre tour de jouer
            printf("A vous de jouer !\nQuel coup voulez-vous faire (x y) ?: ");
            //test du coup demande

            fgets(ligne_envoyee, LIGNE_MAX, stdin);
            lgEcr = ecrireLigne(sock, ligne_envoyee);
            // printf("%d octets ecrits\n", lgEcr);

            //Affichage grille
            system("clear");
            afficherGrille();
        }

        

        
        // lireLigne(sock, ligne_recue);
        // printf("%s\n", ligne_recue);

    }


    // fermeture de la connexion

    if(close(ret) == -1) {
        erreur_IO("fermeture canal");
    }
    else {
        printf("Session terminee.\n");
    }

    
    return(0);
}