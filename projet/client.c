/*
Fonctionnalités du client du jeu Morpion
- afficher la grille (grille qui est gérée par le serveur donc il faut un dialogue..)
- acquérir le choix du joueur quand c'est son tour (système de flag pour bloquer lors du tour de l'autre joueur)
- afficher victoire/défaite/match nul puis quitter
*/



#include "pse.h"
//#include "morpion.h"
#define CMD "client"


// #define DEBUT_MATCH 's'
#define TOUR_JOUEUR 't'
#define VICTOIRE    'v'
#define DEFAITE     'd'
#define MATCH_NUL   'n'

// #define FIN_PARTIE  'f' //utile ?



void initialiserGrille(char grille[3][3]) {
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            grille[i][j] = '-';
        }
    }
}

void afficherGrille(char grille[3][3]) {
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            printf("%c ", grille[i][j]);
        }
        printf("\n");
    }
}





int main(int argc, char *argv[]) {
     // initialisation du client
    int sock, ret;
    struct sockaddr_in *adrServ;
    char ligne_envoyee[LIGNE_MAX];
    char ligne_recue[LIGNE_MAX];
    int lgEcr;

    char grille_morpion[3][3];

    int match, joueur;

    int x = -1;
    int y = -1;
    int x_dernier_coup_adversaire;
    int y_dernier_coup_adversaire;

    int flag_fin_partie = FAUX;

    char buffer;



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
    

    system("clear");

    // on affiche le numero de match et l'equipe du joueur
    lireLigne(sock, ligne_recue); //recuperation du numero du match joue et du joueur incarne
    sscanf(ligne_recue, "%c %d %d", &buffer, &match, &joueur); //on recupere les numeros depuis la chaine recue


    // printf("%s\n", ligne_recue);
    printf("Match joue: %d, vous etes le joueur n°%d\n", match, joueur);

    // initialisation de la grille de jeu
    initialiserGrille(grille_morpion);
    // affichage de la grille
    // afficherGrille(grille_morpion);

    

    // while(strcmp(ligne_recue, "fin\n") != 0) {
    while(!flag_fin_partie) {
        lireLigne(sock, ligne_recue);


        switch (ligne_recue[0])
        {
        case TOUR_JOUEUR:
            // a notre tour de jouer
            //on recupere le dernier coup joue
            sscanf(ligne_recue, "%c %d %d", &buffer, &x_dernier_coup_adversaire, &y_dernier_coup_adversaire);
            //on update la grille avec le dernier coup de l'adversaire
            grille_morpion[y_dernier_coup_adversaire][x_dernier_coup_adversaire] = (!joueur)? 'o' : 'x';
            //on affiche la grille avec le dernier coup adversaire
            system("clear");
            afficherGrille(grille_morpion);

            //test du coup demande

            // fgets(ligne_envoyee, LIGNE_MAX, stdin);
            
            x = -1;
            y = -1;
            while ( ((x < 0) || (x > 2) || (y < 0) || (y > 2)) || grille_morpion[y][x] != '-') {
                printf("A vous de jouer !\nQuel coup voulez-vous faire (x y) ?: ");
                scanf("%d %d", &x, &y);
            }

            // on communique le coup joue
            sprintf(ligne_envoyee, "c %d %d", x, y);
            lgEcr = ecrireLigne(sock, ligne_envoyee);
            // printf("%d octets ecrits\n", lgEcr);

            // on place le coup sur la grille
            grille_morpion[y][x] = joueur? 'o' : 'x';

            //Affichage grille
            system("clear");
            afficherGrille(grille_morpion);
            break;

        case DEFAITE:
            sscanf(ligne_recue, "%c %d %d", &buffer, &x_dernier_coup_adversaire, &y_dernier_coup_adversaire);
            //on update la grille avec le dernier coup de l'adversaire
            grille_morpion[y_dernier_coup_adversaire][x_dernier_coup_adversaire] = (!joueur)? 'o' : 'x';
            //on affiche la grille avec le dernier coup adversaire
            system("clear");
            afficherGrille(grille_morpion);

            printf("Défaite ...\n");
            flag_fin_partie = VRAI;
            break;

        case VICTOIRE:
            sscanf(ligne_recue, "%c %d %d", &buffer, &x_dernier_coup_adversaire, &y_dernier_coup_adversaire);
            //on update la grille avec le dernier coup de l'adversaire
            grille_morpion[y_dernier_coup_adversaire][x_dernier_coup_adversaire] = (!joueur)? 'o' : 'x';
            //on affiche la grille avec le dernier coup adversaire
            system("clear");
            afficherGrille(grille_morpion);

            printf("Victoire !!!\n");
            flag_fin_partie = VRAI;
            break;

        case MATCH_NUL:
            sscanf(ligne_recue, "%c %d %d", &buffer, &x_dernier_coup_adversaire, &y_dernier_coup_adversaire);
            //on update la grille avec le dernier coup de l'adversaire
            grille_morpion[y_dernier_coup_adversaire][x_dernier_coup_adversaire] = (!joueur)? 'o' : 'x';
            //on affiche la grille avec le dernier coup adversaire
            system("clear");
            afficherGrille(grille_morpion);

            printf("Match nul\n");
            flag_fin_partie = VRAI;
            break;
        
        default:
            break;
        }

      

    }


    // fermeture de la connexion

    if(close(ret) == -1) {
        erreur_IO("fermeture canal");
    }
    else {
        printf("Partie terminée.\n");
    }
    
    
    return(0);
}