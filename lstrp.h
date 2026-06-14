#ifndef LSTRP_H
#define LSTRP_H

#include "liste.h"

NOEUD noeudCreer(ELEMENT e);
void noeudDetruire(NOEUD n);
LISTE listeCreer(void);
void listeDetruire(LISTE);
int estVide(LISTE);
int estSaturee(LISTE);
int listeTaille(LISTE);
ELEMENT recuperer(LISTE, int);
int inserer(LISTE, ELEMENT, int);
int supprimer(LISTE, int);
void listeAfficher(LISTE);
LISTE listeCopier(LISTE);
int listeComparer(LISTE, LISTE);

#endif

