#ifndef _ELTARTICLE_H
#define _ELTARTICLE_H

typedef struct {
    int  id;
    char titre[100];
    char source[50];
    int  score_fiabilite;
    int  jour, mois, annee;
    int  heure, minute;
} articleStruct, *ELEMENT;

#define ELEMENT_VIDE NULL

ELEMENT creerArticle(void);
void lireArticle(ELEMENT *);
void afficherArticle(ELEMENT);
void affecterArticle(ELEMENT *, ELEMENT);
void copierArticle(ELEMENT *, ELEMENT);
int  comparerArticle(ELEMENT, ELEMENT);
void detruireArticle(ELEMENT);

#endif


