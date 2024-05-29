/*
Fonctionnalités du serveur de jeu Morpion
(- Gérer les parties simultanées)
- Ordonner les tours de chaque joueur
- tester la grille et donner la victoire/defaite/match nul
- acquerir les coup des joueurs (le test de legalite peut etre fait par le client)



Astuce: pour communiquer entre les workers: variables globales !



NB_WORKERS = NB_JOUEURS
*/

#include "pse.h"
// #include "morpion.h"


#define CMD "serveur"
#define NB_JOUEURS 4

DataSpec dataW[NB_JOUEURS]; //structure de controle des threads joueurs sur le serveur

typedef struct
{
    int x;
    int y;
}Coup;


typedef struct Match_t
{
    /*
    Structure definissant les elements principaux d'un match. Certains seront peut etre inutiles
    */
    int match_en_cours;
    int joueur1;
    int joueur2;
    int tour;
    int gagnant;
    Coup dernier_coup_joue;

    char grille[3][3];
} Match;

//tableau a initialiser automatiquement a l'avenir mais qui contient les matchs a jouer
Match matchs[NB_JOUEURS/2] = {
    {0, 0, 1, 0, -1, {-1, -1}, {{'-', '-', '-'}, {'-', '-', '-'}, {'-', '-', '-'}}},
    {0, 2, 3, 0, -1, {-1, -1}, {{'-', '-', '-'}, {'-', '-', '-'}, {'-', '-', '-'}}}
};


int match_courant = 0;
int joueur_courant = 0;
int nb_joueurs_connectes = 0;

char buffer;
//flags de controle des actions serveur. Ils sont declenches par les workers
int flag_check_grille = FAUX;
int flag_debut_jeu = FAUX;
int flag_message_debut_jeu = NB_JOUEURS;


void* worker(void* arg);
void init_workers(void);
int available_worker(void);
int verifierGagnant(char grille[3][3]);












int main(int argc, char *argv[]) {
    printf("[Serveur de tournoi de morpion]\n\n");

    // initialisation des workers
    init_workers();

    // initialisation du serveur
    short port;
    int ecoute, canal, ret;
    struct sockaddr_in adrEcoute, adrClient;
    unsigned int lgAdrClient;
    
    if (argc != 2)
        erreur("usage: %s port\n", argv[0]);

    port = (short)atoi(argv[1]);

    printf("%s: creating a socket\n", CMD);
    ecoute = socket (AF_INET, SOCK_STREAM, 0);
    if (ecoute < 0)
        erreur_IO("socket");
    
    adrEcoute.sin_family = AF_INET;
    adrEcoute.sin_addr.s_addr = INADDR_ANY;
    adrEcoute.sin_port = htons(port);
    printf("%s: binding to INADDR_ANY address on port %d\n", CMD, port);
    ret = bind (ecoute,  (struct sockaddr *)&adrEcoute, sizeof(adrEcoute));
    if (ret < 0)
        erreur_IO("bind");
    
   


    while (1) {
        // connexion de tout les joueurs avant debut de partie

        while (nb_joueurs_connectes < NB_JOUEURS) {
            // attente de connexion
            printf("%s: listening to socket\n", CMD);
            ret = listen (ecoute, 5);
            if (ret < 0)
            erreur_IO("listen");

            printf("%s: accepting a connection\n", CMD);
            lgAdrClient = sizeof(adrClient);
            canal = accept(ecoute, (struct sockaddr *)&adrClient, &lgAdrClient); //fonction qui attends une nouvelle connexion
            if (canal < 0)
                erreur_IO("accept");

            printf("%s: adr %s, port %hu\n", CMD,
                stringIP(ntohl(adrClient.sin_addr.s_addr)),
                ntohs(adrClient.sin_port));

            //affectation d'un worker a la connexion client
            // start_worker(canal);
            //recherche d'un thread disponible
            int i = available_worker();

            if (i != NB_JOUEURS) {
                //on affecte la gestion du client au worker i
                printf("Match %d: joueur %d connecte.\n", match_courant, joueur_courant);
                dataW[i].canal = canal;
                dataW[i].match = match_courant;
                dataW[i].joueur = joueur_courant;

                sem_post(&dataW[i].sem); //on actionne le semaphore
                
                nb_joueurs_connectes++;

                //controle de l'affectation aux matchs et joueurs des workers (renommes en joueur)
                if (joueur_courant < 1) joueur_courant++;
                else {
                    joueur_courant = 0;
                    match_courant++;
                }
            }        
            else {
                //gestion de la saturation
                printf("Tout les workers sont occupés !\n"); //a retirer prochainement
            }
        }

        //flag_debut_jeu = VRAI; //flag attendu par tout les workers pour demarrer de maniere synchronisee
        for(int i=0; i<NB_JOUEURS; i++) {
            matchs[i].match_en_cours = 1;
        }


        // attente de la fin des matchs (tout les canaux à -1)
        int a = 0;

        while( a != NB_JOUEURS*-1 ) {
            a = 0;
            for(int i=0; i<NB_JOUEURS; i++) {
            a += dataW[i].canal;
            }
            usleep(100000); //100 ms
        }
        

        printf("Fin de la première série de matchs.\n");
        nb_joueurs_connectes = 0; //on reset, c'est temporaire
    }


    return(0);
}





void* worker(void* arg) {
    /*Fonction thread de session client*/
    DataSpec* donnees_thread = (DataSpec* )arg;
    

    //boucle infinie principale
    while (1) {
        //attente d'une assignation de joueur
        sem_wait(&donnees_thread->sem);

        //recuperer les infos de connexion via la structure passee en argument
        int canal = donnees_thread->canal;
        int match_courant = donnees_thread->match;
        int joueur= donnees_thread->joueur;

        char ligne_recue[LIGNE_MAX];
        char ligne_envoyee[LIGNE_MAX];
        int lgLue;

        int x_joue, y_joue;

        

        //affichage du match et du numero de joueur
        sprintf(ligne_envoyee, "i %d %d", match_courant, joueur);
        ecrireLigne(canal, ligne_envoyee);
        
        while(!matchs[match_courant].match_en_cours) {usleep(100);} //attente de debut de partie
        int flag_local_match_en_cours = VRAI;
        // boucle principale de dialogue utilisateur
        while(flag_local_match_en_cours) {
            
            if (matchs[match_courant].tour == joueur) {
                // c'est au tour du client de ce worker de jouer ('t' de debut de chaine)
                // on dit quel est le dernier coup joue sur la partie
                // sprintf(ligne_envoyee, "t %d %d\n", matchs[match].dernier_coup_joue.x, matchs[match].dernier_coup_joue.y);
                // ecrireLigne(canal, ligne_envoyee);

                // on teste si il y a eu victoire d'un joueur
                if (matchs[match_courant].gagnant == joueur) {
                    // victoire du joueur
                    sprintf(ligne_envoyee, "v %d %d\n", matchs[match_courant].dernier_coup_joue.x, matchs[match_courant].dernier_coup_joue.y);
                    ecrireLigne(canal, ligne_envoyee);
                    flag_local_match_en_cours = FAUX; //arret du match
                }
                if (matchs[match_courant].gagnant == !joueur) {
                    // victoire de l'adversaire
                    sprintf(ligne_envoyee, "d %d %d\n", matchs[match_courant].dernier_coup_joue.x, matchs[match_courant].dernier_coup_joue.y);
                    ecrireLigne(canal, ligne_envoyee);
                    flag_local_match_en_cours = FAUX; //arret du match
                }
                if (matchs[match_courant].gagnant == 2) {
                    // match nul
                    sprintf(ligne_envoyee, "n %d %d\n", matchs[match_courant].dernier_coup_joue.x, matchs[match_courant].dernier_coup_joue.y);
                    ecrireLigne(canal, ligne_envoyee);
                    flag_local_match_en_cours = FAUX; //arret du match
                }
                if (matchs[match_courant].gagnant == -1) {
                    // pas de victoire, on joue
                    sprintf(ligne_envoyee, "t %d %d\n", matchs[match_courant].dernier_coup_joue.x, matchs[match_courant].dernier_coup_joue.y);
                    ecrireLigne(canal, ligne_envoyee);
                }
        
                    
                    
                

                // on attends le coup valide joue par le joueur
                lgLue = lireLigne(canal, ligne_recue);
                printf("Serveur. Ligne de %d octet(s) recue: %s\n", lgLue, ligne_recue);
                sscanf(ligne_recue, "%c %d %d", &buffer, &x_joue, &y_joue); //on recupere le coup joue pour l'envoyer au joueur 2
                
                //on sauvegarde le coup joue pour le donner au client du joueur adverse pour update sa grille
                matchs[match_courant].dernier_coup_joue.x = x_joue;
                matchs[match_courant].dernier_coup_joue.y = y_joue;
                matchs[match_courant].grille[y_joue][x_joue] = joueur? 'o' : 'x';

                // on teste si il y a victoire
                switch (verifierGagnant(matchs[match_courant].grille))
                {
                case 10:
                    // victoire joueur 1
                    matchs[match_courant].gagnant = 0;
                    break;
                
                case 11:
                    // victoire joueur 2
                    matchs[match_courant].gagnant = 1;
                    break;

                case -1:
                    // match nul
                    matchs[match_courant].gagnant = 2;
                    break;

                case 0:
                    // pas de victoire
                    break;
                
                default:
                    break;
                }


                //a la fin du tour on switch le tour
                matchs[match_courant].tour ^= 1;
            }
        }


        //fermeture canal
        // if(close(canal) == -1) {
        //     erreur_IO("fermeture canal");
        // }
        // else {
        //     printf("Session terminee.\n");
        // }


        //mise en veille du worker
        donnees_thread->canal = -1;
        
        

    }
    pthread_exit(NULL); //jamais atteint mais la syntaxe le demande
}







void init_workers(void) {
    //initialisation de la cohorte
    for(int i=0; i<NB_JOUEURS; i++) {
        dataW[i].canal = -1; //-1 pour mettre en sommeil le thread
        sem_init(&dataW[i].sem, 0, 0); //initialisation du semaphore worker
        pthread_create(&dataW[i].id, NULL, worker, &dataW[i]); //creation de threads
    }
}



int available_worker(void) {
    int i = 0;
    while(dataW[i].canal != -1 && i < NB_JOUEURS) {i++;}

    return i;
}


int verifierGagnant(char grille[3][3]) {
    // Vérification des lignes et des colonnes
    for(int i = 0; i < 3; i++) {
        if (grille[i][0] == grille[i][1] && grille[i][1] == grille[i][2] && grille[i][0] != '-' && grille[i][0] == 'x') {
            return 10; // victoire joueur 1
        }
        if (grille[i][0] == grille[i][1] && grille[i][1] == grille[i][2] && grille[i][0] != '-' && grille[i][0] == 'o') {
            return 11; // victoire joueur 2
        }

        if (grille[0][i] == grille[1][i] && grille[1][i] == grille[2][i] && grille[0][i] != '-' && grille[0][i] == 'x') {
            return 10; // victoire joueur 1
        }
        if (grille[0][i] == grille[1][i] && grille[1][i] == grille[2][i] && grille[0][i] != '-' && grille[0][i] == 'o') {
            return 11; // victoire joueur 2
        }
    }
    // diagonales
    if ((grille[0][0] == grille[1][1] && grille[1][1] == grille[2][2] && grille[0][0] != '-' && grille[0][0] == 'x') ||
        (grille[0][2] == grille[1][1] && grille[1][1] == grille[2][0] && grille[0][2] != '-'&& grille[0][2] == 'x')) {
        return 10; // victoire
    }
    if ((grille[0][0] == grille[1][1] && grille[1][1] == grille[2][2] && grille[0][0] != '-' && grille[0][0] == 'o') ||
        (grille[0][2] == grille[1][1] && grille[1][1] == grille[2][0] && grille[0][2] != '-'&& grille[0][2] == 'o')) {
        return 11; // victoire
    }

    // grille pleine ?
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            if (grille[i][j] == '-') {
                return 0; // le jeu se poursuit
            }
        }
    }

    //on est sorti des boucles for donc la grille est pleine
    return -1; // match nul...
}