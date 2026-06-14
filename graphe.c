#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FAKESDB.H"
#include "graphe.h"
#include "eltarticle.h"
#include "lstrp.h"

#define ROUGE "\033[31m"
#define VERT "\033[32m"
#define JAUNE "\033[33m"
#define RESET "\033[0m"

grapheReseau creerGraphe(int V) {
  int i;
  grapheReseau g = (grapheReseau)malloc(sizeof(*g));
  if (g == ELEMENT_VIDE) {
    printf(ROUGE "Erreur allocation graphe !" RESET "\n");
    return NULL;
  }
  g->V = V;

  g->articles = (ELEMENT *)calloc(V, sizeof(ELEMENT));
  if (g->articles == NULL) {
    printf(ROUGE "Erreur allocation articles !" RESET "\n");
    free(g);
    return NULL;
  }

  g->adjList = (LISTE *)calloc(V, sizeof(LISTE));
  if (g->adjList == NULL) {
    printf(ROUGE "Erreur allocation adjList !" RESET "\n");
    free(g->articles);
    free(g);
    return NULL;
  }

  g->degre_in = (int *)calloc(V, sizeof(int));
  if (g->degre_in == NULL) {
    printf(ROUGE "Erreur allocation degre_in !" RESET "\n");
    free(g->adjList);
    free(g->articles);
    free(g);
    return NULL;
  }

  for (i = 0; i < V; i++)
    g->adjList[i] = listeCreer();

  return g;
}

grapheReseau chargerGraphe(const char *nomfichier) {
  FILE *fichier = fopen(nomfichier, "r"); //"r" siginife "READ-ONLY permession"
  if (fichier == NULL) {
    printf(ROUGE "Erreur ouverture fichier!" RESET "\n");
    return NULL;
  }

  char lignes[500][256];
  int nbLignes = 0;
  while (fgets(lignes[nbLignes], sizeof(lignes[0]), fichier))
    nbLignes++;

  fclose(fichier);

  int V = 0, i;
  for (i = 0; i < nbLignes; i++)
    if (lignes[i][0] == 'A')
      V++;

  grapheReseau g = creerGraphe(V);

  for (i = 0; i < nbLignes; i++) {
      //ignorer les lignes qui se commencent par #
    if (lignes[i][0] == 'A') {
      int id, score, jour, mois, annee, heure, minute;
      char titre[100], source[50];
      sscanf(lignes[i], "A %d \"%[^\"]\" %s %d %d %d %d %d %d", &id, titre,
             source, &score, &jour, &mois, &annee, &heure, &minute);
      ELEMENT art = creerArticle();
      if (art != ELEMENT_VIDE) {
        art->id = id;
        strncpy(art->titre, titre, sizeof(art->titre) - 1);
        art->titre[sizeof(art->titre) - 1] = '\0';
        strncpy(art->source, source, sizeof(art->source) - 1);
        art->source[sizeof(art->source) - 1] = '\0';
        art->score_fiabilite = score;
        art->jour = jour;
        art->mois = mois;
        art->annee = annee;
        art->heure = heure;
        art->minute = minute;
        g->articles[id] = art;
      }
    } else if (lignes[i][0] == 'C') {
      int src, dest;
      sscanf(lignes[i], "C %d %d", &src, &dest);
      ajouterCitation(g, src, dest);
    }
  }
  return g;
}

void detruireGraphe(grapheReseau g){
    int i;
    for(i = 0; i < g->V; i++){
        listeDetruire(g->adjList[i]);
        detruireArticle(g->articles[i]);
    }
    free(g->adjList);
    free(g->articles);
    free(g->degre_in);
    free(g);
}

int IndexParId(grapheReseau g, int id) {
    if (g == NULL) return -1;
    for (int i = 0; i < g->V; i++) {
        if (g->articles[i] != ELEMENT_VIDE && g->articles[i]->id == id)
            return i;
    }
    return -1;
}

int prochainId(grapheReseau g) {
    if (g == NULL || g->V == 0) return 0;
    int maxId = -1;
    for (int i = 0; i < g->V; i++) {
        if (g->articles[i] != ELEMENT_VIDE && g->articles[i]->id > maxId)
            maxId = g->articles[i]->id;
    }
    return maxId + 1;
}

int ajouterArticle(grapheReseau g, ELEMENT art) {
    if (g == NULL || art == ELEMENT_VIDE) {
        printf(ROUGE "Parametres invalides !" RESET "\n");
        return 0;
    }
    int id = art->id;
    if (id < 0) {
        printf(ROUGE "Erreur : ID doit etre >= 0 !" RESET "\n");
        return 0;
    }
    if (IndexParId(g, id) != -1) {
        printf(JAUNE "L'article %d existe deja !" RESET "\n", id);
        return 0;
    }

    int nouvelleTaille = g->V + 1;

    // realloc sécurisé avec pointeurs temporaires
    ELEMENT *tmp_articles = realloc(g->articles, nouvelleTaille * sizeof(ELEMENT));
    LISTE   *tmp_adjList  = realloc(g->adjList,  nouvelleTaille * sizeof(LISTE));
    int     *tmp_degre_in = realloc(g->degre_in, nouvelleTaille * sizeof(int));

    if (!tmp_articles || !tmp_adjList || !tmp_degre_in) {
        // Si un seul échoue, on libère les autres pour éviter le leak
        if (tmp_articles) g->articles = tmp_articles;
        if (tmp_adjList)  g->adjList  = tmp_adjList;
        if (tmp_degre_in) g->degre_in = tmp_degre_in;
        printf(ROUGE "Erreur d'allocation memoire !" RESET "\n");
        return 0;
    }

    g->articles = tmp_articles;
    g->adjList  = tmp_adjList;
    g->degre_in = tmp_degre_in;

    // Initialiser le nouvel emplacement avant la copie
    g->articles[g->V] = ELEMENT_VIDE;
    copierArticle(&g->articles[g->V], art);

    if (g->articles[g->V] == ELEMENT_VIDE) {
        printf(ROUGE "Erreur copie article !" RESET "\n");
        return 0;
    }

    g->adjList[g->V]  = listeCreer();
    g->degre_in[g->V] = 0;
    g->V = nouvelleTaille;

    printf(VERT "Article N°%d ajoute avec succes !" RESET "\n", id);
    return 1;
}

int supprimerArticle(grapheReseau g, int idArt) {
    if (g == NULL) {
        printf(ROUGE "Graphe invalide !" RESET "\n");
        return 0;
    }

    int i = IndexParId(g, idArt); // ← corrigé (était getIndexParId)
    if (i == -1) {
        printf(ROUGE "Article %d non trouve !" RESET "\n", idArt);
        return 0;
    }

    // Supprimer toutes les arêtes entrantes vers idArt
    // dans les listes d'adjacence de tous les autres sommets
    for (int j = 0; j < g->V; j++) {
        if (j == i || g->adjList[j] == NULL) continue;

        NOEUD courant = g->adjList[j]->tete;
        int k = 1;
        while (courant != NULL) {
            NOEUD suivant = courant->suivant; // ← sauvegarde avant suppression
            if (courant->info != ELEMENT_VIDE && courant->info->id == idArt) {
                supprimer(g->adjList[j], k);
                // k reste identique car les positions ont décalé
            } else {
                k++;
            }
            courant = suivant; // ← utilise le pointeur sauvegardé (pas freed)
        }
    }

    // Détruire la liste d'adjacence du sommet supprimé
    listeDetruire(g->adjList[i]);

    int dernierIndex = g->V - 1;

    if (i != dernierIndex) {
        // Swap : remplacer i par le dernier élément
        g->articles[i] = g->articles[dernierIndex];
        g->adjList[i]  = g->adjList[dernierIndex];
        g->degre_in[i] = g->degre_in[dernierIndex];

        // Recalculer degre_in[i] après le swap
        // car le dernier article a changé de position
        g->degre_in[i] = 0;
        for (int j = 0; j < g->V - 1; j++) {
            if (j == i || g->adjList[j] == NULL) continue;
            NOEUD courant = g->adjList[j]->tete;
            while (courant != NULL) {
                if (courant->info != ELEMENT_VIDE &&
                    courant->info->id == g->articles[i]->id) {
                    g->degre_in[i]++;
                }
                courant = courant->suivant;
            }
        }
    }

    g->V--;

    // Réduire la mémoire
    if (g->V > 0) {
        ELEMENT *tmp_articles = realloc(g->articles, g->V * sizeof(ELEMENT));
        LISTE   *tmp_adjList  = realloc(g->adjList,  g->V * sizeof(LISTE));
        int     *tmp_degre_in = realloc(g->degre_in, g->V * sizeof(int));

        // realloc peut retourner NULL si réduction échoue (rare)
        // mais les anciens pointeurs restent valides dans ce cas
        if (tmp_articles) g->articles = tmp_articles;
        if (tmp_adjList)  g->adjList  = tmp_adjList;
        if (tmp_degre_in) g->degre_in = tmp_degre_in;
    } else {
        free(g->articles); g->articles = NULL;
        free(g->adjList);  g->adjList  = NULL;
        free(g->degre_in); g->degre_in = NULL;
    }

    printf(VERT "Article %d supprime avec succes !" RESET "\n", idArt);
    return 1;
}

int ajouterCitation(grapheReseau g, int idSrc, int idDest){
    if (g == NULL) {
    printf(ROUGE "Graphe invalide !" RESET "\n");
    return 0;
  }

    int indexSrc = IndexParId(g, idSrc);
    int indexDest = IndexParId(g, idDest);

    if (indexSrc == -1 || indexDest == -1) {
    printf(ROUGE "Un des articles n'existe pas !" RESET "\n");
    return 0;
  }

  if (g->adjList[indexSrc] == NULL)
    g->adjList[indexSrc] = listeCreer();
  NOEUD courant = g->adjList[indexSrc]->tete;
  int trouve = 0;
  while (courant != NULL && !trouve) {
      if (courant->info != ELEMENT_VIDE && courant->info->id == idDest) {
          trouve = 1;
      }
      courant = courant->suivant;
  }
  if (trouve) {
      printf(JAUNE "La citation existe deja !" RESET "\n");
      return 0;
  }
  inserer(g->adjList[indexSrc], g->articles[indexDest], g->adjList[indexSrc]->lg + 1);
  g->degre_in[indexDest]++;
  printf(VERT "Citation ajoutee : %d -> %d" RESET "\n", idSrc, idDest);
  return 1;
}

int supprimerCitation(grapheReseau g, int idSrc, int idDest){
  if (g == NULL) {
    printf(ROUGE "Graphe invalide !" RESET "\n");
    return 0;
  }

  int indexSrc = IndexParId(g, idSrc);
  int indexDest = IndexParId(g, idDest);

  if (indexSrc == -1 || indexDest == -1) {
    printf(ROUGE "Un des articles n'existe pas !" RESET "\n");
    return 0;
  }

  if (g->adjList[indexSrc] == NULL || estVide(g->adjList[indexSrc])) {
    printf(ROUGE "Aucune citation a supprimer !" RESET "\n");
    return 0;
  }

  /* Parcourir la liste d'adjacence de indexSrc pour trouver idDest */
  NOEUD courant = g->adjList[indexSrc]->tete;
  int pos = 1;
  while (courant != NULL) {
    if (courant->info != ELEMENT_VIDE && courant->info->id == idDest) {
      supprimer(g->adjList[indexSrc], pos);
      g->degre_in[indexDest]--;
      printf(VERT "Citation supprimee : %d -> %d" RESET "\n", idSrc, idDest);
      return 1;
    }
    courant = courant->suivant;
    pos++;
  }

  printf(JAUNE "Citation %d -> %d non trouvee !" RESET "\n", idSrc, idDest);
  return 0;
}

void afficherGraphe(grapheReseau g) {
  int i;
  if (g == NULL) {
    printf(ROUGE "Graphe invalide !" RESET "\n");
    return;
  }
  printf("=== Affichage Des Articles ===\n");
  for (i = 0; i < g->V; i++) {
    printf("Article %d : ", i);
    afficherArticle(g->articles[i]);
    printf(" || Citations : ");
    listeAfficher(g->adjList[i]);
    printf("\n");
  }
}

void articlesCites(grapheReseau g, int idSrc){
if (g == NULL) { printf(ROUGE "Graphe invalide !" RESET "\n"); return; }
    int idx = IndexParId(g, idSrc);
    if (idx == -1) {
        printf(ROUGE "Article %d n'existe pas !" RESET "\n", idSrc);
        return;
    }
    printf("Les articles cites par l'article %d sont :\n", idSrc);
    NOEUD courant = g->adjList[idx]->tete;
    while (courant != NULL) {
        if (courant->info != ELEMENT_VIDE) {
            printf("  Titre : %s\n",   courant->info->titre);
            printf("  Source : %s\n",  courant->info->source);
            printf("  Score : %d\n",   courant->info->score_fiabilite);
            printf("  Date & Heure: %d/%d/%d %d:%d\n",
                   courant->info->jour, courant->info->mois, courant->info->annee,
                   courant->info->heure, courant->info->minute);
            printf("\n");
        }
        courant = courant->suivant;
    }
}

void articlesCitants(grapheReseau g, int idDest){
    if (g == NULL) { printf(ROUGE "Graphe invalide !" RESET "\n"); return; }
    int idxDest = IndexParId(g, idDest);
    if (idxDest == -1) {
        printf(ROUGE "Article %d n'existe pas !" RESET "\n", idDest);
        return;
    }
    printf("Les articles qui citent l'article %d sont :\n", idDest);
    for (int i = 0; i < g->V; i++) {
        if (i == idxDest || g->adjList[i] == NULL) continue;  // ← index vs index ✓
        NOEUD courant = g->adjList[i]->tete;
        while (courant != NULL) {
            if (courant->info != ELEMENT_VIDE && courant->info->id == idDest) {
                printf("Article %d :\n", g->articles[i]->id);
                printf("    Titre : %s\n",  g->articles[i]->titre);
                printf("    Source : %s\n", g->articles[i]->source);
                printf("    Score : %d\n",  g->articles[i]->score_fiabilite);
                printf("    Date & Heure: %d/%d/%d %d:%d\n",
                       g->articles[i]->jour, g->articles[i]->mois, g->articles[i]->annee,
                       g->articles[i]->heure, g->articles[i]->minute);
                printf("\n");
                break;
            }
            courant = courant->suivant;
        }
    }
}

void sourcesOriginales(grapheReseau g){
    if (g == NULL) {
      printf(ROUGE "Graphe invalide !" RESET "\n");
      return;
    }
    printf("Les sources originales sont :\n");
    for (int i = 0; i < g->V; i++) {
        if (g->articles[i] != ELEMENT_VIDE && estVide(g->adjList[i])) {
            printf("Article %d :\n", i);
            printf("  || Titre : %s\n", g->articles[i]->titre);
            printf("  || Source : %s\n", g->articles[i]->source);
            printf("  || Score : %d\n", g->articles[i]->score_fiabilite);
            printf("  || Date & Heure: %d/%d/%d %d:%d\n", g->articles[i]->jour, g->articles[i]->mois, g->articles[i]->annee, g->articles[i]->heure, g->articles[i]->minute);
            printf("\n");
        }
    }
}

void articlesIsoles(grapheReseau g){ //0 citant , 0 cites
    if (g == NULL) {
      printf(ROUGE "Graphe invalide !" RESET "\n");
      return;
    }
    printf("Les Articles Isolés sont :\n");
    for(int i = 0 ; i<g->V ; i++){
        if(g->articles[i]!= ELEMENT_VIDE && g->degre_in[i] == 0 && estVide(g->adjList[i])){
            printf("Article %d : %s \n", i, g->articles[i]->titre);
        }

    }
}

ELEMENT articlePlusCite(grapheReseau g){
    if (g == NULL) {
      printf(ROUGE "Graphe invalide !" RESET "\n");
      return ELEMENT_VIDE;
    }
    ELEMENT e = ELEMENT_VIDE;
    int maxDegre = -1;
    for(int i = 0 ; i < g->V ; i++){
        if(g->articles[i] != ELEMENT_VIDE && g->degre_in[i] > maxDegre){
            maxDegre = g->degre_in[i];
            e = g->articles[i];
        }
    }
    return e;
}



void trierParDate(grapheReseau g){
    for (int i = 1; i < g->V; i++) {
        ELEMENT tempArt  = g->articles[i];
        LISTE   tempList = g->adjList[i];
        int     tempDeg  = g->degre_in[i];

        int j = i - 1;
        while (j >= 0&& g->articles[j] != ELEMENT_VIDE && tempArt != ELEMENT_VIDE && comparerArticle(g->articles[j], tempArt) > 0) {
            g->articles[j+1] = g->articles[j];
            g->adjList[j+1]  = g->adjList[j];
            g->degre_in[j+1] = g->degre_in[j];
            j--;
        }
        g->articles[j+1] = tempArt;
        g->adjList[j+1]  = tempList;
        g->degre_in[j+1] = tempDeg;
    }
}

void premierCitant(grapheReseau g, int idDest) {
    if (g == NULL) {
        printf(ROUGE "Graphe invalide !" RESET "\n");
        return;
    }
    int idxDest = IndexParId(g, idDest);
    if (idxDest == -1) {
        printf(ROUGE "Article %d n'existe pas !" RESET "\n", idDest);
        return;
    }

    ELEMENT plusAncien  = ELEMENT_VIDE;
    int     idxPlusAncien = -1;

    for (int i = 0; i < g->V; i++) {
        if (i == idxDest || g->adjList[i] == NULL) continue;

        NOEUD courant = g->adjList[i]->tete;
        while (courant != NULL) {
            if (courant->info != ELEMENT_VIDE && courant->info->id == idDest) {
                // Premier trouvé OU plus ancien que le candidat actuel
                if (plusAncien == ELEMENT_VIDE ||
                    comparerArticle(g->articles[i], plusAncien) < 0) {
                    plusAncien    = g->articles[i];
                    idxPlusAncien = i;
                }
            }
            courant = courant->suivant;
        }
    }

    if (idxPlusAncien == -1) {
        printf(JAUNE "Aucun article citant trouve pour l'article %d." RESET "\n", idDest);
        return;
    }

    printf("Premier article citant l'article %d :\n", idDest);
    printf("    Article  : %d\n",    plusAncien->id);
    printf("    Titre    : %s\n",    plusAncien->titre);
    printf("    Source   : %s\n",    plusAncien->source);
    printf("    Score    : %d\n",    plusAncien->score_fiabilite);
    printf("    Date     : %02d/%02d/%d %02d:%02d\n", plusAncien->jour, plusAncien->mois, plusAncien->annee,plusAncien->heure, plusAncien->minute);
}


void chainePropagation(grapheReseau g, int idSrc) {
    // Reconstitue et affiche la chaîne chronologique de propagation depuis l'article idSrc
    // affiche les articles dans l'ordre où ils ont été publiés en citant un article déjà dans la chaîne.

    if (g == NULL) {
        printf(ROUGE "Graphe invalide !" RESET "\n");
        return;
    }
    if (idSrc < 0 || idSrc >= g->V) {
        printf(ROUGE "ID Hors Limite !" RESET "\n");
        return;
    }
    if (g->articles[idSrc] == ELEMENT_VIDE) {
        printf(ROUGE "Article %d n'existe pas !" RESET "\n", idSrc);
        return;
    }

    // Allocation des structures pour le parcours en largeur (BFS)
    //Voir Cours "SRP" page 6 -> 13 <parcours des graphes>
    int *file = (int *)malloc(g->V * sizeof(int));      // File pour le BFS
    int *visite = (int *)calloc(g->V, sizeof(int));     // Tableau des articles déjà visités (0 ou 1)
    ELEMENT *chainePropagation = (ELEMENT *)malloc(g->V * sizeof(ELEMENT)); // Contient les articles de la chaîne

    if (!file || !visite || !chainePropagation) { //validation d'allocation
        printf(ROUGE "Erreur d'allocation mémoire !" RESET "\n");
        if (file) free(file);
        if (visite) free(visite);
        if (chainePropagation) free(chainePropagation);
        return;
    }

    int debut = 0, fin = 0;
    int nbArticles = 0;

    // Ajouter l'article de base à la propagation
    file[fin++] = idSrc;
    visite[idSrc] = 1;

    // Parcours pour trouver tous les descendants de propagation
    while (debut < fin) {
        int curr = file[debut++];
        chainePropagation[nbArticles++] = g->articles[curr];

        // Parcourir tous les articles pour trouver ceux qui citent l'article 'curr'
        for (int i = 0; i < g->V; i++) {
            if (g->adjList[i] == NULL) continue;
            NOEUD courant = g->adjList[i]->tete;
            int trouve = 0;
            while (courant != NULL && !trouve) {
                if (courant->info != ELEMENT_VIDE && courant->info->id == g->articles[curr]->id && !visite[i]) {
                    file[fin++] = i;
                    visite[i] = 1;
                    trouve = 1;
                }
                courant = courant->suivant;
            }
        }
    }

    // Tri de la chaîne par ordre chronologique de publication (Tri à bulles)
    // On utilise la fonction comparerArticle qui retourne l'ordre de deux dates.
    for (int i = 0; i < nbArticles - 1; i++) {
        for (int j = 0; j < nbArticles - i - 1; j++) {
            if (comparerArticle(chainePropagation[j], chainePropagation[j + 1]) > 0) {
                // Si l'article j est plus récent que l'article j+1, on les échange
                ELEMENT temp = chainePropagation[j];
                chainePropagation[j] = chainePropagation[j + 1];
                chainePropagation[j + 1] = temp;
            }
        }
    }

    // Affichage des articles dans l'ordre chronologique
    printf("Chaîne de propagation depuis l'article %d (%d articles concernés) :\n\n", idSrc, nbArticles);
    for (int i = 0; i < nbArticles; i++) {
        printf(" %d. Article %d | Date: %02d/%02d/%d %02d:%02d\n",
               i + 1, chainePropagation[i]->id,
               chainePropagation[i]->jour, chainePropagation[i]->mois, chainePropagation[i]->annee,
               chainePropagation[i]->heure, chainePropagation[i]->minute);
        printf("    Titre  : %s\n", chainePropagation[i]->titre);
        printf("    Source : %s\n", chainePropagation[i]->source);
        printf("    Score  : %d\n", chainePropagation[i]->score_fiabilite);
        printf("\n");
    }

    // Libération de la mémoire allouée
    free(file);
    free(visite);
    free(chainePropagation);
}

void simulerPropagation(grapheReseau g, int idSrc) {

    if (g == NULL) {
        printf(ROUGE "Graphe invalide !" RESET "\n");
        return;
    }
    if (idSrc < 0 || idSrc >= g->V) {
        printf(ROUGE "ID Hors Limite !" RESET "\n");
        return;
    }
    if (g->articles[idSrc] == ELEMENT_VIDE) {
        printf(ROUGE "Article %d n'existe pas !" RESET "\n", idSrc);
        return;
    }
    //BFS
    int *file = (int *)malloc(g->V * sizeof(int));           // File (queue) pour le BFS
    int *visite = (int *)calloc(g->V, sizeof(int));          // Tableau booléen : 1 si déjà visité
    double *scoreProp = (double *)calloc(g->V, sizeof(double)); // Score de propagation pour chaque article
    int *niveau = (int *)calloc(g->V, sizeof(int));          // Niveau (profondeur) dans le BFS

    if (!file || !visite || !scoreProp || !niveau) {
        printf(ROUGE "Erreur d'allocation mémoire !" RESET "\n");
        if (file) free(file);
        if (visite) free(visite);
        if (scoreProp) free(scoreProp);
        if (niveau) free(niveau);
        return;
    }

  //  Initialisation du BFS
    int debut = 0, fin = 0;  // debut = tête de la file, fin = queue de la file

    file[fin++] = idSrc;                                    // Enfiler l'article source
    visite[idSrc] = 1;                                      // Marquer comme visité
    scoreProp[idSrc] = g->articles[idSrc]->score_fiabilite; // Le score initial = score de fiabilité de la source
    niveau[idSrc] = 0;                                      // La source est au niveau 0

    printf("=== Simulation de propagation depuis l'article %d ===\n\n", idSrc);


    while (debut < fin) {
        // Défiler l'article courant
        int curr = file[debut++];

        // Afficher l'article courant avec son score de propagation
        printf("  Niveau %d | Article %d\n", niveau[curr], curr);
        printf("    Titre  : %s\n", g->articles[curr]->titre);
        printf("    Source : %s\n", g->articles[curr]->source);
        printf("    Score fiabilite : %d\n", g->articles[curr]->score_fiabilite);
        printf("    Score propagation : %.2f\n\n", scoreProp[curr]);
        for (int i = 0; i < g->V; i++) {
            if (visite[i] || g->adjList[i] == NULL) continue; //deja visite
            NOEUD courant = g->adjList[i]->tete;
            int trouve = 0;
            while (courant != NULL && !trouve) {
                // Si l'article i cite l'article curr
                if (courant->info != ELEMENT_VIDE && courant->info->id == g->articles[curr]->id) {
                    // Calculer le score de propagation :
                    // = score du parent * (fiabilité de l'enfant / 100)
                    scoreProp[i] = scoreProp[curr] * (g->articles[i]->score_fiabilite / 100.0);
                    niveau[i] = niveau[curr] + 1;  // Un niveau plus profond

                    file[fin++] = i;   // Enfiler l'article i
                    visite[i] = 1;     // Marquer comme visité
                    trouve = 1;
                }
                courant = courant->suivant;
            }
        }
    }

    int nbAtteints = fin;  // Nombre total d'articles atteints par la propagation
    printf("Nombre total d'articles atteints : %d\n", nbAtteints);

    if (scoreProp[idSrc] < 50) {
        printf(ROUGE "ATTENTION : L'article source a un faible score (%d). "
               "La propagation de fausses informations est probable !" RESET "\n",
               g->articles[idSrc]->score_fiabilite);
    } else {
        printf(VERT "L'article source a un bon score de fiabilité (%d)." RESET "\n",
               g->articles[idSrc]->score_fiabilite);
    }

    free(file);
    free(visite);
    free(scoreProp);
    free(niveau);
}


void articlesAccessibles(grapheReseau g, int idSrc) {
    if (g == NULL) {
        printf(ROUGE "Graphe invalide !" RESET "\n");
        return;
    }
    if (idSrc < 0 || idSrc >= g->V) {
        printf(ROUGE "ID Hors Limite !" RESET "\n");
        return;
    }
    if (g->articles[idSrc] == ELEMENT_VIDE) {
        printf(ROUGE "Article %d n'existe pas !" RESET "\n", idSrc);
        return;
    }

    // Allocation des structures pour le BFS
    int *file = (int *)malloc(g->V * sizeof(int));       // File pour le BFS
    int *visite = (int *)calloc(g->V, sizeof(int));      // Tableau des articles deja visites

    if (!file || !visite) {
        printf(ROUGE "Erreur d'allocation memoire !" RESET "\n");
        if (file) free(file);
        if (visite) free(visite);
        return;
    }

    int debut = 0, fin = 0;

    // Enfiler l'article source
    file[fin++] = idSrc;
    visite[idSrc] = 1;

    // BFS : on suit les citations sortantes (adjList[curr])
    while (debut < fin) {
        int curr = file[debut++];
        if (g->adjList[curr] == NULL) continue;
        // Parcourir la liste d'adjacence de l'article courant
        // adjList[curr] contient les articles cites par curr
        NOEUD courant = g->adjList[curr]->tete;
        while (courant != NULL) {
            if (courant->info != ELEMENT_VIDE) {
                int idxVoisin = IndexParId(g, courant->info->id);
                if (idxVoisin != -1 && !visite[idxVoisin]) {
                    visite[idxVoisin] = 1;
                    file[fin++] = idxVoisin;
                }
            }
            courant = courant->suivant;
        }
    }

    // Affichage des articles accessibles (sauf la source elle-meme)
    int nbAccessibles = 0;
    printf("=== Articles accessibles depuis l'article %d ===\n\n", idSrc);
    for (int i = 0; i < g->V; i++) {
        if (visite[i] && g->articles[i] != ELEMENT_VIDE) {
            printf("  Article %d :\n", i);
            printf("    Titre  : %s\n", g->articles[i]->titre);
            printf("    Source : %s\n", g->articles[i]->source);
            printf("    Score  : %d\n", g->articles[i]->score_fiabilite);
            printf("    Date   : %02d/%02d/%d %02d:%02d\n\n",
                   g->articles[i]->jour, g->articles[i]->mois, g->articles[i]->annee,
                   g->articles[i]->heure, g->articles[i]->minute);
            nbAccessibles++;
        }
    }

    if (nbAccessibles == 0)
        printf(JAUNE "Aucun article accessible depuis l'article %d." RESET "\n", idSrc);
    else
        printf("Nombre total d'articles accessibles : %d\n", nbAccessibles);

    free(file);
    free(visite);
}


int analyserArticle(ELEMENT article) {
    if (article == ELEMENT_VIDE) return 0;

    //Titre en minuscules
    char titreLower[100];
    int k;
    for (k = 0; k < 99 && article->titre[k] != '\0'; k++)
        titreLower[k] = tolower((char)article->titre[k]);
    titreLower[k] = '\0';
    for (k = 0; titreLower[k] != '\0'; k++)
        if (titreLower[k] == '_') titreLower[k] = ' ';

    //score_suspicion = (nb_fakes x 40) + (nb_mots_suspects x 10)
    int nb_fakes        = 0;
    int nb_mots_suspects = 0;

    for (int i = 0; i < NB_FAKES; i++)
        if (strstr(titreLower, obtenirBaseFake(i)) != NULL)
            nb_fakes++;

    for (int i = 0; i < NB_SUSPECTS; i++)
        if (strstr(titreLower, obtenirMotSuspect(i)) != NULL)
            nb_mots_suspects++;

    int score_suspicion = (nb_fakes * 40) + (nb_mots_suspects * 10);

    /* score_fiabilite = max(0, 100 - score_suspicion) */
    int score = 100 - score_suspicion;

    if (score < 0) {
        article->score_fiabilite = 0;
    } else {
        article->score_fiabilite = score;
    }

    return score_suspicion;
}


void analyserReseau(grapheReseau g) {
    if (g == NULL) {
        printf(ROUGE "Graphe invalide !\n" RESET);
        return;
    }

    printf("=== Analyse du reseau ===\n\n");
    int nbSuspect = 0, nbDouteux = 0, nbFiable = 0;

    for (int i = 0; i < g->V; i++) {
        if (g->articles[i] == ELEMENT_VIDE) continue;

        analyserArticle(g->articles[i]);
        int score = g->articles[i]->score_fiabilite;

        //score < 40   -> SUSPECT
        // score < 70   -> DOUTEUX
        //score >= 70  -> FIABLE
        if (score < 40) {
            printf(ROUGE "[SUSPECT] " RESET);
            nbSuspect++;
        } else if (score < 70) {
            printf(JAUNE "[DOUTEUX] " RESET);
            nbDouteux++;
        } else {
            printf(VERT  "[FIABLE ] " RESET);
            nbFiable++;
        }
        printf("%s (score: %d)\n", g->articles[i]->titre, score);

        char titreLower[100];
        int k;
        for (k = 0; k < 99 && g->articles[i]->titre[k] != '\0'; k++)
            titreLower[k] = tolower((unsigned char)g->articles[i]->titre[k]);
        titreLower[k] = '\0';
        for (k = 0; titreLower[k] != '\0'; k++)
            if (titreLower[k] == '_') titreLower[k] = ' ';

        for (int j = 0; j < NB_FAKES; j++)
            if (strstr(titreLower, obtenirBaseFake(j)) != NULL)
                printf(ROUGE "    [FAKE]    \"%s\"\n" RESET, obtenirBaseFake(j));

        for (int j = 0; j < NB_SUSPECTS; j++)
            if (strstr(titreLower, obtenirMotSuspect(j)) != NULL)
                printf(JAUNE "    [SUSPECT] \"%s\"\n" RESET, obtenirMotSuspect(j));

        printf("\n");
    }

    printf("--- RESULTAT D ANALYSE ---\n");
    printf(ROUGE "SUSPECT : %d\n" RESET, nbSuspect);
    printf(JAUNE "DOUTEUX : %d\n" RESET, nbDouteux);
    printf(VERT  "FIABLE  : %d\n" RESET, nbFiable);
}

void articlesSuspectsCites(grapheReseau g) {
    if (g == NULL) {
        printf(ROUGE "Graphe invalide !" RESET "\n");
        return;
    }

    int *suspects = (int *)malloc(g->V * sizeof(int));
    if (!suspects) {
        printf(ROUGE "Erreur d'allocation memoire !" RESET "\n");
        return;
    }
    int nb = 0;

    for (int i = 0; i < g->V; i++) {
        if (g->articles[i] == ELEMENT_VIDE) continue;

        char titreLower[100];
        int k;
        for (k = 0; k < 99 && g->articles[i]->titre[k] != '\0'; k++)
            titreLower[k] = tolower((char)g->articles[i]->titre[k]);
        titreLower[k] = '\0';
        for (k = 0; titreLower[k] != '\0'; k++)
            if (titreLower[k] == '_') titreLower[k] = ' ';

        int estSuspect = 0;

        for (int j = 0; j < NB_FAKES && !estSuspect; j++)
            if (strstr(titreLower, obtenirBaseFake(j)) != NULL)
                estSuspect = 1;

        for (int j = 0; j < NB_SUSPECTS && !estSuspect; j++)
            if (strstr(titreLower, obtenirMotSuspect(j)) != NULL)
                estSuspect = 1;

        if (!estSuspect && g->articles[i]->score_fiabilite < 40)
            estSuspect = 1;

        if (estSuspect && g->degre_in[i] > 0)
            suspects[nb++] = i;
    }

    // Tri à bulles par degre_in décroissant
    for (int i = 0; i < nb - 1; i++) {
        for (int j = 0; j < nb - i - 1; j++) {
            if (g->degre_in[suspects[j]] < g->degre_in[suspects[j + 1]]) {
                int tmp         = suspects[j];
                suspects[j]     = suspects[j + 1];
                suspects[j + 1] = tmp;
            }
        }
    }

    printf("=== Articles suspects qui sont cites (propagation potentielle) ===\n\n");

    if (nb == 0) {
        printf(VERT "Aucun article suspect cite dans le reseau." RESET "\n");
        free(suspects);
        return;
    }

    for (int i = 0; i < nb; i++) {
        int idx = suspects[i];
        printf(ROUGE "  Article %d : %s" RESET "\n",
               g->articles[idx]->id, g->articles[idx]->titre);
        printf("    Source : %s\n",  g->articles[idx]->source);
        printf("    Score  : %d\n",  g->articles[idx]->score_fiabilite);
        printf("    Cite par %d article(s)\n\n", g->degre_in[idx]);
    }

    printf(ROUGE "ATTENTION : %d article(s) suspect(s) propagent "
           "potentiellement de fausses informations !" RESET "\n", nb);

    free(suspects);
}

void simulerSuppression(grapheReseau g, int idArt) {
    if (g == NULL) {
        printf(ROUGE "Graphe invalide !" RESET "\n");
        return;
    }
    if (idArt < 0 || idArt >= g->V) {
        printf(ROUGE "ID Hors Limite !" RESET "\n");
        return;
    }
    if (g->articles[idArt] == ELEMENT_VIDE) {
        printf(ROUGE "Article %d n'existe pas !" RESET "\n", idArt);
        return;
    }

    printf("=== Simulation de suppression de l'article %d ===\n", idArt);
    printf("    Titre : %s\n\n", g->articles[idArt]->titre);

    // Articles qui perdraient une citation (cites par idArt)
    printf("--- Articles qui perdraient une citation entrante ---\n");
    int nbCitesAffectes = 0;
    NOEUD courant = g->adjList[idArt]->tete;
    while (courant != NULL) {
        if (courant->info != ELEMENT_VIDE) {
            int idCite = courant->info->id;
            printf("  Article %d : %s (degre_in actuel : %d -> %d)\n",
                   idCite, g->articles[idCite]->titre,
                   g->degre_in[idCite], g->degre_in[idCite] - 1);
            nbCitesAffectes++;
        }
        courant = courant->suivant;
    }
    if (nbCitesAffectes == 0)
        printf("  Aucun article affecte.\n");

    printf("\n");

    //Articles qui perdraient un citant (qui citent idArt)
    printf("--- Articles qui perdraient un citant ---\n");
    int nbCitantsAffectes = 0;
    for (int i = 0; i < g->V; i++) {
        if (i == idArt || g->adjList[i] == NULL) continue;

        NOEUD c = g->adjList[i]->tete;
        while (c != NULL) {
            if (c->info != ELEMENT_VIDE && c->info->id == idArt) {
                printf("  Article %d : %s (perdrait la citation vers %d)\n",
                       i, g->articles[i]->titre, idArt);
                nbCitantsAffectes++;
                break;
            }
            c = c->suivant;
        }
    }
    if (nbCitantsAffectes == 0)
        printf("  Aucun article affecte.\n");

    printf("\n");

    // Articles qui sont isoles apres suppression
    printf("--- Articles qui deviendraient potentiellement isoles ---\n");
    int nbIsoles = 0;
    courant = g->adjList[idArt]->tete;
    while (courant != NULL) {
        if (courant->info != ELEMENT_VIDE) {
            int idCite = courant->info->id;
            // Si cet article n'a qu'une seule citation entrante (idArt)
            // et il ne cite personne, il deviendrait isole
            if (g->degre_in[idCite] == 1 && estVide(g->adjList[idCite])) {
                printf(JAUNE "  Article %d : %s deviendrait ISOLE !" RESET "\n",
                       idCite, g->articles[idCite]->titre);
                nbIsoles++;
            }
        }
        courant = courant->suivant;
    }
    if (nbIsoles == 0)
        printf("  Aucun article ne deviendrait isole.\n");

    printf("\n--- Resume de la simulation ---\n");
    printf("Citations sortantes supprimees : %d\n", nbCitesAffectes);
    printf("Citations entrantes perdues    : %d\n", nbCitantsAffectes);
    printf("Articles potentiellement isoles: %d\n", nbIsoles);
}


int neutraliserPropagation(grapheReseau g, int idSrc, int idDest) {
    if (g == NULL) {
            printf(ROUGE "Graphe invalide !" RESET "\n");
    return 0; }

    int idxSrc  = IndexParId(g, idSrc);
    int idxDest = IndexParId(g, idDest);

    if (idxSrc == -1 || idxDest == -1) {
        printf(ROUGE "Un des articles n'existe pas !" RESET "\n");
        return 0;
    }

    // Vérifier l'existence de la citation
    NOEUD courant = g->adjList[idxSrc]->tete;
    int trouve = 0;
    while (courant != NULL) {
        if (courant->info != ELEMENT_VIDE && courant->info->id == idDest) {
            trouve = 1;
            break;//arreter l'execution
        }
        courant = courant->suivant;
    }
    if (!trouve) {
        printf(ROUGE "Citation %d -> %d non trouvee !" RESET "\n", idSrc, idDest);
        return 0;
    }

    printf("=== Neutralisation de la propagation : %d -> %d ===\n\n", idSrc, idDest);

    // BFS avant — compter les accessibles depuis idxDest
    int *visite_avant = (int *)calloc(g->V, sizeof(int));
    int *file = (int *)malloc(g->V * sizeof(int));
    if (!visite_avant || !file) {
        printf(ROUGE "Erreur d'allocation memoire !" RESET "\n");
        free(visite_avant); free(file); return 0;
    }
    int debut = 0, fin = 0, nbAvant = 0;
    file[fin++] = idxDest;
    visite_avant[idxDest] = 1;
    while (debut < fin) {
        int curr = file[debut++];
        nbAvant++;
        NOEUD c = g->adjList[curr]->tete;
        while (c != NULL) {
            if (c->info != ELEMENT_VIDE) {
                int idxV = IndexParId(g, c->info->id);
                if (idxV != -1 && !visite_avant[idxV]) {
                    visite_avant[idxV] = 1;
                    file[fin++] = idxV;
                }
            }
            c = c->suivant;
        }
    }

    // Suppression de la citation (sans printf)
    NOEUD c2 = g->adjList[idxSrc]->tete;
    int pos = 1;
    while (c2 != NULL) {
        if (c2->info != ELEMENT_VIDE && c2->info->id == idDest) {
            supprimer(g->adjList[idxSrc], pos);
            g->degre_in[idxDest]--;
            break;
        }
        c2 = c2->suivant;
        pos++;
    }

    // BFS après
    int *visite_apres = (int *)calloc(g->V, sizeof(int));
    if (!visite_apres) {
        printf(ROUGE "Erreur d'allocation memoire !" RESET "\n");
        free(visite_avant); free(file); return 0;
    }
    debut = fin = 0;
    int nbApres = 0;
    file[fin++] = idxDest;
    visite_apres[idxDest] = 1;
    while (debut < fin) {
        int curr = file[debut++];
        nbApres++;
        NOEUD c = g->adjList[curr]->tete;
        while (c != NULL) {
            if (c->info != ELEMENT_VIDE) {
                int idxV = IndexParId(g, c->info->id);
                if (idxV != -1 && !visite_apres[idxV]) {
                    visite_apres[idxV] = 1;
                    file[fin++] = idxV;
                }
            }
            c = c->suivant;
        }
    }

    printf("--- Impact de la neutralisation ---\n");
    printf("Articles accessibles depuis %d AVANT : %d\n", idDest, nbAvant);
    printf("Articles accessibles depuis %d APRES : %d\n", idDest, nbApres);
    printf("Propagation reduite de %d article(s)\n", nbAvant - nbApres);

    if (g->degre_in[idxDest] == 0)
        printf(JAUNE "L'article %d n'est plus cite par personne !" RESET "\n", idDest);

    printf(VERT "Propagation neutralisee avec succes : %d -> %d" RESET "\n", idSrc, idDest);

    free(visite_avant); free(visite_apres); free(file);
    return 1;
}


