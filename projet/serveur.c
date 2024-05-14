#include "pse.h"

#define CMD "serveur"
#define NB_WORKERS 5

DataSpec dataW[NB_WORKERS];

int fd_log;

void init_log(void);
void* worker(void* arg);
void init_workers(void);
int available_worker(void);
void start_worker(int canal);












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
    
    
    //initialisation du log
    init_log();
    
   






    while (1) {
        
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
        start_worker(canal);
        

    }

    if(close(fd_log) == -1) {
        erreur_IO("fermeture journal.log");
    }


    return(0);
}










void init_log(void) {
    // initialisation du log
    fd_log = open("journal.log", O_WRONLY|O_APPEND, 0644);
    if(fd_log == -1) {
        erreur_IO("ouverture journal.log");
    }
    ecrireLigne(fd_log, "Initialisation du journal log.\n");
}



void init_workers(void) {
    //initialisation de la cohorte
    for(int i=0; i<NB_WORKERS; i++) {
        dataW[i].canal = -1; //-1 pour mettre en sommeil le thread
        sem_init(&dataW[i].sem, 0, 0); //initialisation du semaphore worker
        pthread_create(&dataW[i].id, NULL, worker, &dataW[i]); //creation de threads

        
    }
}



int available_worker(void) {
    int i = 0;
    while(dataW[i].canal != -1 && i < NB_WORKERS) {i++;}

    return i;
}



void* worker(void* arg) {
    /*Fonction thread de session client*/
    DataSpec* donnees_thread = (DataSpec* )arg;
    

    //boucle infinie principale
    while (1) {
        //attente d'une assignation de service
        // while (donnees_thread->canal == -1) {
        //     usleep(10000); //tempo de 1000 micro secondes
        // }
        sem_wait(&donnees_thread->sem);

        //gestion du service client
        // //recuperer les infos de connexion via la structure passee en argument
        int canal = donnees_thread->canal;

        char ligne[LIGNE_MAX];
        int lgLue;

        // boucle principale de dialogue utilisateur
        while(strcmp(ligne, "fin") != 0) {

            lgLue = lireLigne(canal, ligne);

            printf("Serveur. Ligne de %d octet(s) recue: %s\n", lgLue, ligne);
            ecrireLigne(fd_log, ligne);
            

            if(lgLue == 0) {
                break;
            }
            if(lgLue == -1) {
                erreur_IO("lecture du message");
            }
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


void start_worker(int canal) {
    //recherche d'un thread disponible
    int i = available_worker();

    if (i != NB_WORKERS) {
        //on affecte la gestion du client au worker i
        printf("Je suis le worker %d, a votre service.\n", i);
        dataW[i].canal = canal;
        sem_post(&dataW[i].sem); //on actionne le semaphore
    }        
    else {
        //gestion de la saturation
        printf("Tout les workers sont occupés !\n");
    }
}