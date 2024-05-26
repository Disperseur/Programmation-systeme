/*
Fonctionnalités du client du jeu Morpion
- afficher la grille (grille qui est gérée par le serveur donc il faut un dialogue..)
- acquérir le choix du joueur quand c'est son tour (système de flag pour bloquer lors du tour de l'autre joueur)
- afficher victoire/défaite/match nul puis quitter
*/



#include "pse.h"
//#include "morpion.h"
#define CMD "client"


#define DEBUT_MATCH 's'
#define TOUR_JOUEUR 't'
#define FIN_PARTIE  'f'



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

int verifierGagnant(char grille[3][3]) {
    // Vérification des lignes et des colonnes
    for(int i = 0; i < 3; i++) {
        if (grille[i][0] == grille[i][1] && grille[i][1] == grille[i][2] && grille[i][0] != '-') {
            return 1; // victoire
        }
        if (grille[0][i] == grille[1][i] && grille[1][i] == grille[2][i] && grille[0][i] != '-') {
            return 1; // victoire
        }
    }
    // diagonales
    if ((grille[0][0] == grille[1][1] && grille[1][1] == grille[2][2] && grille[0][0] != '-') ||
        (grille[0][2] == grille[1][1] && grille[1][1] == grille[2][0] && grille[0][2] != '-')) {
        return 1; // victoire
    }
    // grille pleine ?
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            if (grille[i][j] == '-') {
                return 0; // le jeu se poursuit
            }
        }
    }
    return -1; // match nul...
}



int main(int argc, char *argv[]) {
     // initialisation du client
    int sock, ret;
    struct sockaddr_in *adrServ;
    char ligne_envoyee[LIGNE_MAX];
    char ligne_recue[LIGNE_MAX];
    int lgEcr;

    char grille_morpion[3][3];

    int match, joueur, gagnant;

    int x = -1;
    int y = -1;
    int x_dernier_coup_adversaire;
    int y_dernier_coup_adversaire;




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
    lireLigne(sock, ligne_recue); //recuperation du numero du match joue et du joueur incarne
    sscanf(ligne_recue, "%d %d", &match, &joueur); //on recupere les numeros depuis la chaine recue


    // printf("%s\n", ligne_recue);
    printf("Match joue: %d, vous etes le joueur n°%d\n", match, joueur);

    // initialisation de la grille de jeu
    initialiserGrille(grille_morpion);
    // affichage de la grille
    // afficherGrille(grille_morpion);

    

    // while(strcmp(ligne_recue, "fin\n") != 0) {
    while(ligne_recue[0] != FIN_PARTIE) {
        lireLigne(sock, ligne_recue);

        if (ligne_recue[0] == DEBUT_MATCH) {
            printf("Debut de la partie !\n");
        }

        if (ligne_recue[0] == TOUR_JOUEUR) {
        
             
            // a notre tour de jouer
            //on recupere le dernier coup joue
            char buffer;
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
                printf("C'est à votre tour de jouer !\nQuel coup voulez-vous faire (x y) ?: ");
                scanf("%d %d", &x, &y);
            }

            // on communique le coup joue
            sprintf(ligne_envoyee, "%d %d", x, y);
            lgEcr = ecrireLigne(sock, ligne_envoyee);
            // printf("%d octets ecrits\n", lgEcr);

            // on place le coup sur la grille
            grille_morpion[y][x] = joueur? 'o' : 'x';
            
            if (verifierGagnant(grille_morpion) == 1) {
            		 system("clear");
		         lireLigne(sock, ligne_recue);
            		 sscanf(ligne_recue, "Victoire du joueur %d !!!!\n", &gagnant);
            		 printf("Victoire du joueur %d !!!!\n", gagnant);
		         break;
		     } else if (verifierGagnant(grille_morpion) == -1) {
		     	 system("clear");
		         printf("Match nul...\n");
		         break;
		     }
            

            //Affichage grille
            system("clear");
            afficherGrille(grille_morpion);
            
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