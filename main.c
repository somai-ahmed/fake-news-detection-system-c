#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "eltarticle.h"
#include "graphe.h"


#ifdef _WIN32
    #include <windows.h>
    #define SLEEP_MS(ms) Sleep(ms)
    #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
        #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
    #endif
    #define clears() system("cls")
    void setupWindowsTerminal() {
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            DWORD dwMode = 0;
            if (GetConsoleMode(hOut, &dwMode)) {
                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hOut, dwMode);
            }
        }
    }
#else
    #include <unistd.h>
    #define SLEEP_MS(ms) usleep((ms) * 1000)
    #define clears() system("clear")
    void setupWindowsTerminal(void) {}
#endif

#define ROUGE "\033[31m"
#define VERT  "\033[32m"
#define JAUNE "\033[33m"
#define CYAN  "\033[36m"
#define BOLD  "\033[1m"
#define RESET "\033[0m"

void typewriter(const char *texte, int delaiMs) {
    int i;
    for (i = 0; texte[i] != '\0'; i++) {
        putchar(texte[i]);
        fflush(stdout);
        SLEEP_MS(delaiMs);
    }
}

void spinner(const char *message, int frames) {
    char anim[] = {'|', '/', '-', '\\'};
    int i;
    for (i = 0; i < frames; i++) {
        printf("\r%s %c", message, anim[i % 4]);
        fflush(stdout);
        SLEEP_MS(120);
    }
    printf("\r%s " VERT "✅" RESET "       \n", message);
}

void viderBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int lireEntier(const char *message) {
    int valeur;
    printf(CYAN "%s" RESET, message);
    scanf("%d", &valeur);
    viderBuffer();
    return valeur;
}

void lireChaine(const char *message, char *buffer, size_t taille) {
    printf(CYAN "%s" RESET, message);
    if (fgets(buffer, (int)taille, stdin) != NULL)
        buffer[strcspn(buffer, "\n")] = '\0';
}

void lireChaineValide(const char *message, char *buffer, size_t taille) {
    int contientLettre;
    do {
        contientLettre = 0;
        lireChaine(message, buffer, taille);
        if (buffer[0] == '\0') printf(ROUGE "erreur chaine !" RESET);

        for (int i = 0; buffer[i] != '\0'; i++)
            if (isalpha(buffer[i])) { contientLettre = 1; break; }

        if (!contientLettre)
            printf(ROUGE "  Erreur : doit contenir des caracteres...\n" RESET);

    } while (buffer[0] == '\0' || !contientLettre);
}

int lireEntierBorne(const char *message, int min, int max) {
    int valeur;
    do {
        valeur = lireEntier(message);
        if (valeur < min || valeur > max)
            printf(ROUGE "  Erreur : entrez un nombre entre %d et %d.\n" RESET, min, max);
    } while (valeur < min || valeur > max);
    return valeur;
}

void afficherMenu() {
    printf("\n");
    printf(BOLD CYAN "  ┌──────────────────────────────────────────┐\n");
    printf("  │            MENU PRINCIPAL                │\n");
    printf("  └──────────────────────────────────────────┘\n" RESET);
    printf("\n");
    printf("  " BOLD "--- Gestion du reseau ---" RESET "\n");
    printf("   1.  Charger un reseau depuis un fichier\n");
    printf("   2.  Ajouter un article\n");
    printf("   3.  Ajouter une citation\n");
    printf("   4.  Supprimer un article\n");
    printf("   5.  Supprimer une citation\n");
    printf("   6.  Afficher le reseau\n");
    printf("\n");
    printf("  " BOLD "--- Recherche & Analyse ---" RESET "\n");
    printf("   7.  Articles cites par un article\n");
    printf("   8.  Articles qui citent un article\n");
    printf("   9.  Sources originales / articles isoles\n");
    printf("  10.  Article le plus cite\n");
    printf("  11.  Trier par date de publication\n");
    printf("  12.  Premier article citant\n");
    printf("\n");
    printf("  " BOLD "--- Propagation ---" RESET "\n");
    printf("  13.  Chaine de propagation\n");
    printf("  14.  Simuler la propagation (BFS)\n");
    printf("  15.  Articles accessibles depuis un article\n");
    printf("\n");
    printf("  " BOLD "--- Detection de Fake News ---" RESET "\n");
    printf("  16.  Analyser les articles (fake news)\n");
    printf("  17.  Articles suspects les plus cites\n");
    printf("\n");
    printf("  " BOLD JAUNE "--- BONUS ---" RESET "\n");
    printf("  18.  Simuler la suppression d'un article\n");
    printf("  19.  Neutraliser une propagation\n");
    printf("\n");
    printf("   " ROUGE "0.  Quitter" RESET "\n");
    printf("\n");
    printf(CYAN "  ──────────────────────────────────────────\n" RESET);
}

grapheReseau chargerOuRemplacer(grapheReseau g) {
    char fichier[260];
    grapheReseau nouveau;

    lireChaine("  Nom du fichier : ", fichier, sizeof(fichier));
    spinner("  Chargement du fichier", 25);

    nouveau = chargerGraphe(fichier);
    if (nouveau == NULL) {
        printf(ROUGE "  Chargement echoue.\n" RESET);
        return g;
    }

    if (g != NULL) detruireGraphe(g);

    typewriter(VERT "  Reseau charge avec succes !\n" RESET, 15);
    return nouveau;
}

void ajouterArticleViaMenu(grapheReseau g) {
    int id, score, jour, mois, annee, heure, minute;
    char titre[100];
    char source[50];
    ELEMENT art;

    printf(BOLD "\n  --- Saisie d'un nouvel article ---\n" RESET);

    id = prochainId(g);
    printf(CYAN "  ID assigne automatiquement : " VERT "%d\n" RESET, id);

    lireChaineValide("  Titre  : ", titre, sizeof(titre));
    lireChaineValide("  Source : ", source, sizeof(source));

    score  = lireEntierBorne("  Score de fiabilite [0-100] : ", 0, 100);
    jour   = lireEntierBorne("  Jour   [1-31]      : ", 1, 31);
    mois   = lireEntierBorne("  Mois   [1-12]      : ", 1, 12);
    annee  = lireEntierBorne("  Annee  [2000-2027] : ", 2000, 2027);
    heure  = lireEntierBorne("  Heure  [0-23]      : ", 0, 23);
    minute = lireEntierBorne("  Minute [0-59]      : ", 0, 59);

    art = creerArticle();
    if (art == ELEMENT_VIDE) {
        printf(ROUGE "  Erreur lors de la creation de l'article !\n" RESET);
        return;
    }

    art->id = id;
    strncpy(art->titre,  titre,  sizeof(art->titre)  - 1);
    strncpy(art->source, source, sizeof(art->source) - 1);
    art->titre [sizeof(art->titre)  - 1] = '\0';
    art->source[sizeof(art->source) - 1] = '\0';
    art->score_fiabilite = score;
    art->jour   = jour;
    art->mois   = mois;
    art->annee  = annee;
    art->heure  = heure;
    art->minute = minute;

    spinner("  Ajout en cours", 15);

    if (ajouterArticle(g, art))
        typewriter(VERT "  Article ajoute avec succes !\n" RESET, 15);
    else
        printf(ROUGE "  Erreur lors de l'ajout de l'article !\n" RESET);

    detruireArticle(art);
}

int main() {
    setupWindowsTerminal();

    grapheReseau g = creerGraphe(50);
    int choix;

    if (g == NULL) {
        printf(ROUGE "Impossible d'initialiser le programme.\n" RESET);
        return 1;
    }

    do {
        afficherMenu();
        choix = lireEntier("  Votre choix >> ");
        printf("\n");

        switch (choix) {

        case 1:
            if (g == NULL) break;
            g = chargerOuRemplacer(g);
            break;

        case 2:
            if (g == NULL) break;
            ajouterArticleViaMenu(g);
            break;

        case 3: {
            if (g == NULL) break;
            int src  = lireEntier("  ID article source : ");
            int dest = lireEntier("  ID article cite   : ");
            spinner("  Ajout de la citation", 12);
            ajouterCitation(g, src, dest);
            break;
        }

        case 4: {
            if (g == NULL) break;
            int id = lireEntier("  ID article a supprimer : ");
            spinner("  Suppression en cours", 15);
            supprimerArticle(g, id);
            break;
        }

        case 5: {
            if (g == NULL) break;
            int src  = lireEntier("  ID article source : ");
            int dest = lireEntier("  ID article cite   : ");
            spinner("  Suppression de la citation", 12);
            supprimerCitation(g, src, dest);
            break;
        }

        case 6: {
            if (g == NULL) break;
            spinner("  Chargement du reseau", 10);
            afficherGraphe(g);
            break;
        }

        case 7: {
            if (g == NULL) break;
            articlesCites(g, lireEntier("  ID article : "));
            break;
        }

        case 8: {
            if (g == NULL) break;
            articlesCitants(g, lireEntier("  ID article : "));
            break;
        }

        case 9: {
            if (g == NULL) break;
            printf(BOLD "  --- Sources originales ---\n" RESET);
            sourcesOriginales(g);
            printf(BOLD "\n  --- Articles isoles ---\n" RESET);
            articlesIsoles(g);
            break;
        }

        case 10: {
            if (g == NULL) break;
            ELEMENT art = articlePlusCite(g);
            if (art != ELEMENT_VIDE)
                printf(VERT "  --> %s" RESET " (cite par %d article(s))\n",
                       art->titre, g->degre_in[art->id]);
            else
                printf(JAUNE "  Aucun article.\n" RESET);
            break;
        }

        case 11: {
            if (g == NULL) break;
            if (g->V < 2) {
                printf(JAUNE "  Au moins 2 articles necessaires pour trier.\n" RESET);
                break;
            }
            spinner("  Tri en cours", 15);
            trierParDate(g);
            printf(BOLD "\n  --- Reseau trie par date ---\n" RESET);
            afficherGraphe(g);
            break;
        }

        case 12: {
            if (g == NULL) break;
            premierCitant(g, lireEntier("  ID article cible : "));
            break;
        }

        case 13: {
            if (g == NULL) break;
            chainePropagation(g, lireEntier("  ID article source : "));
            break;
        }

        case 14: {
            if (g == NULL) break;
            simulerPropagation(g, lireEntier("  ID article source : "));
            break;
        }

        case 15: {
            if (g == NULL) break;
            articlesAccessibles(g, lireEntier("  ID article source : "));
            break;
        }

        case 16: {
            if (g == NULL) break;
            spinner("  Analyse en cours", 20);
            analyserReseau(g);
            break;
        }

        case 17: {
            if (g == NULL) break;
            articlesSuspectsCites(g);
            break;
        }

        case 18: {
            if (g == NULL) break;
            simulerSuppression(g, lireEntier("  ID article a tester : "));
            break;
        }

        case 19: {
            if (g == NULL) break;
            int src  = lireEntier("  ID article source      : ");
            int dest = lireEntier("  ID article destination : ");
            spinner("  Neutralisation en cours", 18);
            neutraliserPropagation(g, src, dest);
            break;
        }

        case 0:
            printf("\n");
            typewriter(JAUNE "  Fermeture du programme...\n" RESET, 25);
            spinner("  Nettoyage de la memoire", 15);
            typewriter(VERT "  Au revoir !\n\n" RESET, 30);
            break;

        default:
            printf(ROUGE "  Choix invalide. Reessayez.\n" RESET);
            break;
        }

        if (choix != 0) {
            printf("\n" CYAN "  Appuyez sur Entree pour continuer..." RESET);
            getchar();
            clears();
        }

    } while (choix != 0);

    detruireGraphe(g);
    return 0;
}
