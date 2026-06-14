#ifndef GRAPHE_H
#define GRAPHE_H

#include "eltarticle.h"
#include "liste.h"

typedef struct {
  int V; // nombre de sommets
  ELEMENT *articles; // tableau d'articles
  LISTE *adjList; // liste d'adjacence
  int *degre_in; // degre entrant
} *grapheReseau;

grapheReseau creerGraphe(int V);
grapheReseau chargerGraphe(const char *filename);
void detruireGraphe(grapheReseau g);

int prochainId(grapheReseau g);
int ajouterArticle(grapheReseau g, ELEMENT art);
int supprimerArticle(grapheReseau g, int idArt);
int ajouterCitation(grapheReseau g, int idSrc, int idDest);
int supprimerCitation(grapheReseau g, int idSrc, int idDest);
void afficherGraphe(grapheReseau g);

void articlesCites(grapheReseau g, int idSrc);
void articlesCitants(grapheReseau g, int idDest);
void sourcesOriginales(grapheReseau g);
void articlesIsoles(grapheReseau g);
ELEMENT articlePlusCite(grapheReseau g);

void trierParDate(grapheReseau g);
void premierCitant(grapheReseau g, int idDest);
void chainePropagation(grapheReseau g, int idSrc);

void simulerPropagation(grapheReseau g, int idSrc);
void articlesAccessibles(grapheReseau g, int idSrc);

int analyserArticle(ELEMENT article);
void analyserReseau(grapheReseau g);
void articlesSuspectsCites(grapheReseau g);

void simulerSuppression(grapheReseau g, int idArt);
int neutraliserPropagation(grapheReseau g, int idSrc, int idDest);


int paires(grapheReseau g , int i,int j,int seuil);
void chainecitationssuspectes(grapheReseau g , int seuil);

#endif

