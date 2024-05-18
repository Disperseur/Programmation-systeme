#include "pse.h"
#define CMD "client"


int main(int argc, char *argv[]) {
     // initialisation du client
    int sock, ret;
    struct sockaddr_in *adrServ;
    char ligne_envoyee[LIGNE_MAX];
    char ligne_recue[LIGNE_MAX];
    int lgEcr;

    if (argc != 3)
        erreur("usage: %s machine port\n", argv[0]);

    printf("%s: creating a socket\n", CMD);
    sock = socket (AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        erreur_IO("socket");

    printf("%s: DNS resolving for %s, port %s\n", CMD, argv[1], argv[2]);
    adrServ = resolv(argv[1], argv[2]);
    if (adrServ == NULL)
        erreur("adresse %s port %s inconnus\n", argv[1], argv[2]);

    printf("%s: adr %s, port %hu\n", CMD,
            stringIP(ntohl(adrServ->sin_addr.s_addr)),
            ntohs(adrServ->sin_port));

    printf("%s: connecting the socket\n", CMD);
    ret = connect(sock, (struct sockaddr *)adrServ, sizeof(struct sockaddr_in));
    if (ret < 0)
    erreur_IO("connect");

    // boucle principale
    //
    while(strcmp(ligne_envoyee, "fin\n") != 0) {
        printf("> ");
        fgets(ligne_envoyee, LIGNE_MAX, stdin);
        lgEcr = ecrireLigne(sock, ligne_envoyee);
        printf("%d octets ecrits\n", lgEcr);
        lireLigne(sock, ligne_recue);
        printf("%s\n", ligne_recue);

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