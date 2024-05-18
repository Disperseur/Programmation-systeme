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
#include "morpion.h"


#define CMD "serveur"
#define NB_JOUEURS 2

DataSpec dataW[NB_JOUEURS]; //structure de controle des threads joueurs sur le serveur

typedef struct Match_t
{
    /*
    Structure definissant les elements principaux d'un match. Certains seront peut etre inutiles
    */
    int joueur1;
    int joueur2;
    int tour;
    int gagnant;
    char grille[3][3];
} Match;

//tableau a initialiser automatiquement a l'avenir mais qui contient les matchs a jouer
Match matchs[NB_JOUEURS/2] = {
    {0, 1, 0, -1, {{'-', '-', '-'}, {'-', '-', '-'}, {'-', '-', '-'}}},
    //{2, 3, -1, {{'-', '-', '-'}, {'-', '-', '-'}, {'-', '-', '-'}}}
};


int match_courant = 0;
int joueur_courant = 0;
int nb_joueurs_connectes = 0;

//flags de controle des actions serveur. Ils sont declenches par les workers
int flag_check_grille = FAUX;
int flag_start_jeu = FAUX;


void* worker(void* arg);
void init_workers(void);
int available_worker(void);












int main(int argc, char *argv[]) {

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

        while (nb_joueurs_connectes < 2) {
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
                printf("Je suis le worker %d, a votre service.\n", i);
                dataW[i].canal = canal;
                dataW[i].match = match_courant;
                dataW[i].joueur = joueur_courant;

                sem_post(&dataW[i].sem); //on actionne le semaphore
                
                nb_joueurs_connectes++;

                //controle de l'affectation aux matchs et joueurs des workers (renommes en joueur)
                if (joueur_courant < 2) joueur_courant++;
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

        flag_start_jeu = VRAI; //flag attendu par tout les workers pour demarrer de maniere synchronisee
        


        //Gestion des events
        if (flag_check_grille) {
            //verification des grilles de jeu


            flag_check_grille = 0; //reset du flag
        }
    }

    // if(close(fd_log) == -1) {
    //     erreur_IO("fermeture journal.log");
    // }


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
        int match = donnees_thread->match;
        int joueur= donnees_thread->joueur;

        char ligne_recue[LIGNE_MAX];
        char ligne_envoyee[LIGNE_MAX];
        int lgLue;

        //affichage du match et du numero de joueur
        sprintf(ligne_envoyee, "match n°%d, joueur n°%d\n", match, joueur);
        ecrireLigne(canal, ligne_envoyee);
        

        // boucle principale de dialogue utilisateur
        while(strcmp(ligne_recue, "fin") != 0) {
            
            if (flag_start_jeu) {
            
                if (matchs[match].tour == joueur) {
                    // c'est au tour du client de ce worker de jouer
                    sprintf(ligne_envoyee, "t\n");
                    ecrireLigne(canal, ligne_envoyee);

                    
                    lgLue = lireLigne(canal, ligne_recue);
                    printf("Serveur. Ligne de %d octet(s) recue: %s\n", lgLue, ligne_recue);


                    //a la fin du tour on switch le tour
                    matchs[match].tour ^= 1;
                }
            }



         

            
            

            // if(lgLue == 0) {
            //     break;
            // }
            // if(lgLue == -1) {
            //     erreur_IO("lecture du message");
            // }
        }


        //fermeture canal et thread
        if(close(canal) == -1) {
            erreur_IO("fermeture canal");
        }
        else {
            printf("Session terminee.\n");
        }


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


