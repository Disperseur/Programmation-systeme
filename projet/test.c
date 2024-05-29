#include <stdio.h>
#include <stdlib.h>

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





int main() {
    char grille[3][3] = {'-', '-', '-',
                         'o', 'o', 'o',
                         '-', '-', '-'};

    printf("%d\n", verifierGagnant(grille));

    return(0);
}