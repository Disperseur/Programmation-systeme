#ifndef MORPION_H
#define MORPION_H

#include <stdio.h>
#include <stdlib.h>

char grille[3][3];

void initialiserGrille();
void afficherGrille();
int verifierGagnant();
int partie_morpion();


#endif