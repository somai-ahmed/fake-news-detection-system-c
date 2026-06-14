#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eltarticle.h"

ELEMENT creerArticle() {
    ELEMENT art = (ELEMENT)malloc(sizeof(articleStruct));
    if (art == ELEMENT_VIDE) {
        printf("ERREUR MEMOIRE !\n");
        return ELEMENT_VIDE;
    }
    art->id              = 0;
    art->titre[0]        = '\0';
    art->source[0]       = '\0';
    art->score_fiabilite = 0;
    art->jour            = 0;
    art->mois            = 0;
    art->annee           = 0;
    art->heure           = 0;
    art->minute          = 0;
    return art;
}

void lireArticle(ELEMENT *art) {
    if (*art == ELEMENT_VIDE)
        *art = creerArticle();
    if (*art == ELEMENT_VIDE) return;

    printf("ID : ");
    scanf("%d",  &(*art)->id);

    printf("Titre : ");
    scanf(" %99[^\n]",  (*art)->titre);

    printf("Source : ");
    scanf(" %49[^\n]",  (*art)->source);

    printf("Score [0-100]: ");
    scanf("%d",  &(*art)->score_fiabilite);

    printf("Jour : ");
    scanf("%d",  &(*art)->jour);

    printf("Mois : ");
    scanf("%d",  &(*art)->mois);

    printf("Annee : ");
    scanf("%d",  &(*art)->annee);

    printf("Heure : ");
    scanf("%d",  &(*art)->heure);

    printf("Minute : ");
    scanf("%d",  &(*art)->minute);
}

void afficherArticle(ELEMENT art) {
    if (art == ELEMENT_VIDE) {
        printf("(article vide)\n");
        return;
    }
    printf("%s (id:%d, source:%s, score:%d, %02d/%02d/%d %02d:%02d)\n",
           art->titre, art->id, art->source, art->score_fiabilite,
           art->jour, art->mois, art->annee, art->heure, art->minute);
}


//on elemine la generalite des fonction (comme e1 , e2)
//on a declare deux variable : src (indiquant le source d'article) et dest (indiquant la destination d'article)
void copierArticle(ELEMENT *dest, ELEMENT src) {
    if (src == ELEMENT_VIDE) {
        *dest = ELEMENT_VIDE;
        return;
    }
    *dest = creerArticle();
    if (*dest == ELEMENT_VIDE) return;

    (*dest)->id              = src->id;
    (*dest)->score_fiabilite = src->score_fiabilite;
    (*dest)->jour            = src->jour;
    (*dest)->mois            = src->mois;
    (*dest)->annee           = src->annee;
    (*dest)->heure           = src->heure;
    (*dest)->minute          = src->minute;
    strncpy((*dest)->titre,  src->titre,  sizeof((*dest)->titre)  - 1);
    strncpy((*dest)->source, src->source, sizeof((*dest)->source) - 1);
    (*dest)->titre [sizeof((*dest)->titre)  - 1] = '\0';
    (*dest)->source[sizeof((*dest)->source) - 1] = '\0';
}
//l'affectage rassemble au copiage mais dans l'affectage les deux variable pointe sur la meme adresse
//(dest) et (src) partagent le même objet en mémoire
void affecterArticle(ELEMENT *dest, ELEMENT src) {
    *dest = src;
}

int comparerArticle(ELEMENT art1, ELEMENT art2) {
    if (art1 == ELEMENT_VIDE || art2 == ELEMENT_VIDE)
        return 0;
    if (art1->annee  != art2->annee)
      return art1->annee  - art2->annee;

    if (art1->mois   != art2->mois)
      return art1->mois   - art2->mois;

    if (art1->jour   != art2->jour)
      return art1->jour   - art2->jour;

    if (art1->heure  != art2->heure)
      return art1->heure  - art2->heure;

    return art1->minute - art2->minute;
}

void detruireArticle(ELEMENT art) {
    if (art != ELEMENT_VIDE)
        free(art);
}
