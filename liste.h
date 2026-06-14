#ifndef LISTE_H
#define LISTE_H

#include "eltarticle.h"

typedef struct structNoeud {
    ELEMENT info;
    struct structNoeud * suivant;
}structNoeud, * NOEUD;

typedef struct {
    NOEUD tete;
    int lg;
} laStruct,*LISTE;

LISTE listeCreer(void);
void listeDetruire(LISTE l);
int estVide(LISTE l);
int inserer(LISTE l, ELEMENT e, int position);
int supprimer(LISTE l, int position);
void listeAfficher(LISTE l);

#endif

