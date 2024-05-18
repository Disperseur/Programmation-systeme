#include "morpion.h"

char grille[3][3];

void initialiserGrille() {
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            grille[i][j] = '-';
        }
    }
}

void afficherGrille() {
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            printf("%c ", grille[i][j]);
        }
        printf("\n");
    }
}

int verifierGagnant() {
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

int partie_morpion() {
    int ligne, colonne, joueur = 1;
    int tour = 0;
    char marque;

    initialiserGrille();
    system("clear");
    printf("***Tournoi de morpion***\n\n");
    afficherGrille();
    printf("\n");

    while(tour<9) {
        joueur = (joueur % 2) ? 1 : 2;

        printf("Joueur %d, entre la ligne et la colonne : ", joueur);
        scanf("%d %d", &ligne, &colonne);

        marque = (joueur == 1) ? 'X' : 'O';

        if (ligne < 0 || ligne > 2 || colonne < 0 || colonne > 2 || grille[ligne][colonne] != '-') {
            printf("Coup invalide, recommence\n");
            joueur--;
        } else {
            grille[ligne][colonne] = marque;
            tour++;

            system("clear");
            printf("***Tournoi de morpion***\n\n");
            afficherGrille();
            printf("\n");

            if (verifierGagnant() == 1) {
                printf("Victoire du joueur %d !!!!\n", joueur);
                return(joueur);
                break;
            } else if (verifierGagnant() == -1) {
                printf("Match nul...\n");
                return(-1);
                break;
            }
        }
        joueur++;
    }

    return 0;
}
