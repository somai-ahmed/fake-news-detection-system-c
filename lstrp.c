#include "eltarticle.h"
#include "LSTRP.H"
#include <stdio.h>
#include <stdlib.h>

NOEUD noeudCreer(ELEMENT e) {
  NOEUD n = (NOEUD)malloc(sizeof(structNoeud));
  if (n == NULL) {
    printf("ERREUR DE MEMOIRE !\n");
    return NULL;
  }
  copierArticle(&n->info, e);
  n->suivant = NULL;
  return n;
}

void noeudDetruire(NOEUD n) {
  if (n != NULL) {
    detruireArticle(n->info);
    free(n);
  }
}

LISTE listeCreer(void) {
  LISTE L = (LISTE)malloc(sizeof(laStruct));
  if (!L)
    printf("\nProbleme de memoire");
  else {
    L->lg = 0;
    L->tete = NULL;
  }
  return L;
}

void listeDetruire(LISTE L) {
  int i;
  NOEUD p, q;
  q = L->tete;
  for (i = 1; i <= L->lg; i++) {
    p = q;
    q = q->suivant;
    noeudDetruire(p);
  }
  free(L);
}

int estVide(LISTE L) { return (L->lg == 0); }

int estSaturee(LISTE L) {
  (void)L;
  NOEUD temp = (NOEUD)malloc(sizeof(structNoeud));
  int saturee = 1;
  if (temp != NULL) {
    saturee = 0;
    free(temp);
  }
  return saturee;
}

int listeTaille(LISTE L) { return L->lg; }

ELEMENT recuperer(LISTE L, int pos) {
  int i;
  NOEUD p;
  if (estVide(L))
    printf("\nListe vide");
  else {
    if ((pos < 1) || (pos > L->lg))
      printf("\nPosition invalide");
    else {
      p = L->tete;
      for (i = 1; i < pos; i++)
        p = p->suivant;
      return p->info;
    }
  }
  return ELEMENT_VIDE;
}

int inserer(LISTE l, ELEMENT e, int pos) {
  int i;
  NOEUD p, q;
  if ((pos < 1) || (pos > l->lg + 1)) {
    printf("\nPosition invalide");
    return 0;
  }
  q = (NOEUD)malloc(sizeof(structNoeud));
  if (q == NULL) {
    printf("\nErreur d'allocation");
    return 0;
  }
  affecterArticle(&q->info, e);
  if (pos == 1) {
    q->suivant = l->tete;
    l->tete = q;
  } else {
    p = l->tete;
    for (i = 1; i < pos - 1; i++)
      p = p->suivant;
    q->suivant = p->suivant;
    p->suivant = q;
  }
  l->lg++;
  return 1;
}

int supprimer(LISTE L, int pos) {
  int i, succee = 1;
  NOEUD p, q;
  if (estVide(L)) {
    printf("\nListe vide");
    succee = 0;
  } else {
    if ((pos < 1) || (pos > L->lg)) {
      printf("\nPosition invalide");
      succee = 0;
    } else {
      q = L->tete;
      if (pos == 1)
        L->tete = L->tete->suivant;
      else {
        for (i = 1; i < pos; i++) {
          p = q;
          q = q->suivant;
        }
        p->suivant = q->suivant;
      }
      noeudDetruire(q);
      L->lg--;
    }
  }
  return succee;
}

void listeAfficher(LISTE L) {
  int i;
  NOEUD p = L->tete;
  for (i = 1; i <= L->lg; i++) {
    afficherArticle(p->info);
    p = p->suivant;
  }
}

LISTE listeCopier(LISTE L) {
  LISTE LR = listeCreer();
  int i;
  ELEMENT copie;
  for (i = 1; i <= L->lg; i++) {
    copierArticle(&copie, recuperer(L, i));
    inserer(LR, copie, i);
  }
  return LR;
}

int listeComparer(LISTE L1, LISTE L2) {
  int test = 1, i = 1;
  if (listeTaille(L1) != listeTaille(L2))
    test = 0;
  while ((i <= listeTaille(L1)) && test) {
    if (comparerArticle(recuperer(L1, i), recuperer(L2, i)) != 0)
      test = 0;
    i++;
  }
  return test;
}
