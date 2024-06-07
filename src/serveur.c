#include "pse.h"
// #include "morpion.h"


#define CMD "serveur"




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


DataSpec *dataW; //structure de controle des threads joueurs sur le serveur
Match *matchs; //tableau de structures Matchs pour le suivi des matchs en cours


int match_courant = 0;
int joueur_courant = 0;
int nb_joueurs = 0;
int nb_joueurs_connectes = 0;

char buffer;



void* worker(void* arg);
void init_workers(void);
int available_worker(void);
int verifierGagnant(char grille[3][3]);
void init_matchs(void);











int main(int argc, char *argv[]) {
    printf("[Serveur de matchs de morpion]\n\n");

    printf("Combien de matchs voulez vous créer ? (1-3) : ");
    while (nb_joueurs <= 0 || nb_joueurs >= 4) {
        scanf("%d", &nb_joueurs);
    }
    nb_joueurs*=2;

    printf("%d joueurs\n", nb_joueurs);

    // initialisation des workers
    init_workers();
    init_matchs();

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
    
   


    // connexion de tout les joueurs avant debut de partie

    while (nb_joueurs_connectes < nb_joueurs) {
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
        //recherche d'un thread disponible
        int i = available_worker();

        if (i != nb_joueurs) {
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

    for(int i=0; i<nb_joueurs; i++) {
        matchs[i].match_en_cours = 1;
    }

    printf("Début des matchs !\n\n");


    // attente de la fin des matchs (tout les canaux à -1)
    int a = 0;

    while( a != nb_joueurs*-1 ) {
        a = 0;
        for(int i=0; i<nb_joueurs; i++) {
        a += dataW[i].canal;
        }
        usleep(100000); //100 ms
    }
    

    printf("Fin des matchs !\n");


    return(0);
}





void* worker(void* arg) {
    /*Fonction thread de session client*/
    DataSpec* donnees_thread = (DataSpec* )arg;
    

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
                // c'est au tour du client de jouer

                // on teste si il y a eu victoire d'un joueur
                if (matchs[match_courant].gagnant == joueur) {
                    // victoire du joueur
                    sprintf(ligne_envoyee, "v %d %d\n", matchs[match_courant].dernier_coup_joue.x, matchs[match_courant].dernier_coup_joue.y);
                    ecrireLigne(canal, ligne_envoyee);
                    flag_local_match_en_cours = FAUX; //arret du match
                    printf("Match n°%d: Victoire du joueur %d !\n", match_courant+1, joueur+1);
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
                    printf("Match n°%d: match nul.\n", match_courant+1);
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



        //mise en veille du worker
        donnees_thread->canal = -1;
        
        

    }
    pthread_exit(NULL); //jamais atteint mais la syntaxe le demande
}







void init_workers(void) {
    //initialisation de la cohorte
    dataW = malloc(nb_joueurs*sizeof(DataSpec));

    for(int i=0; i<nb_joueurs; i++) {
        dataW[i].canal = -1; //-1 pour mettre en sommeil le thread
        sem_init(&dataW[i].sem, 0, 0); //initialisation du semaphore worker
        pthread_create(&dataW[i].id, NULL, worker, &dataW[i]); //creation de threads
    }
}



int available_worker(void) {
    int i = 0;
    while(dataW[i].canal != -1 && i < nb_joueurs) {i++;}

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



void init_matchs(void) {
    matchs = malloc(nb_joueurs * sizeof(Match));


    for(int i=0; i<nb_joueurs; i=i+2) {
        // Match *nouveau_match = malloc(sizeof(Match));
        Match nouveau_match = {0, i, i+1, 0, -1, {-1, -1}, {{'-', '-', '-'}, {'-', '-', '-'}, {'-', '-', '-'}}};

        matchs[i/2] = nouveau_match;
    }
}