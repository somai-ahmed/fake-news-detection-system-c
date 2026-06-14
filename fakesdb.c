#include "fakesdb.h"

const char *BASE_FAKES[] = {
    "ils vous cachent", "partagez avant suppression", "ce qu on vous cache",
    "la verite sur", "les medias ne disent pas", "wake up"};

const char *MOTS_SUSPECTS[] = {"alerte", "urgent", "exclusif", "censure",
                                      "complot", "secret", "interdit", "choc"};

const char *obtenirBaseFake(int indice) {
  if (indice < 0 || indice >= NB_FAKES) {
    return "";
  }
  return BASE_FAKES[indice];
}

const char *obtenirMotSuspect(int indice) {
  if (indice < 0 || indice >= NB_SUSPECTS) {
    return "";
  }
  return MOTS_SUSPECTS[indice];
}

