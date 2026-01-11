#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "sen.h"

/* Structure Capteur pour sauvegarder et restaurer */
typedef struct capteur {
    float energie;
    int id_suivant;
    Paquet* buffer;
    int taille_buffer;
    int termine;         // 0 = interruption, 1 = programme terminé
} Capteur;

/* Sauvegarde de l'état dans le fichier save.bin */
void sauvegarder_etat(Capteur *c) {
    FILE *f = fopen("save.bin", "wb");
    if (!f) {
        printf("Erreur ouverture fichier pour sauvegarde\n");
        return;
    }

    fwrite(&c->energie, sizeof(float), 1, f);
    fwrite(&c->id_suivant, sizeof(int), 1, f);
    fwrite(&c->taille_buffer, sizeof(int), 1, f);
    fwrite(&c->termine, sizeof(int), 1, f);

    Paquet* courant = c->buffer;
    while (courant != NULL) {
        fwrite(courant, sizeof(Paquet), 1, f);
        courant = courant->suivant;
    }

    fclose(f);
}

/* Restauration de l'état depuis save.bin */
void restaurer_etat(Capteur *c) {
    FILE *f = fopen("save.bin", "rb");
    if (!f) {
        // Pas de sauvegarde → démarrage normal
        c->energie = 5.0f;
        c->id_suivant = 1;
        c->buffer = NULL;
        c->taille_buffer = 0;
        c->termine = 0;
        return;
    }

    fread(&c->energie, sizeof(float), 1, f);
    fread(&c->id_suivant, sizeof(int), 1, f);
    fread(&c->taille_buffer, sizeof(int), 1, f);
    fread(&c->termine, sizeof(int), 1, f);

    if (c->termine == 1) {
        // Programme terminé → tout remettre à zéro
        c->energie = 5.0f;
        c->id_suivant = 1;
        c->taille_buffer = 0;
        c->buffer = NULL;
        c->termine = 0;
        printf("[INFO] Programme terminé précédemment, redémarrage à zéro avec batterie 20 J.\n");
    } else {
        // Restauration normale du buffer
        Paquet* dernier = NULL;
        for (int i = 0; i < c->taille_buffer; i++) {
            Paquet* p = malloc(sizeof(Paquet));
            fread(p, sizeof(Paquet), 1, f);
            p->suivant = NULL;
            if (dernier == NULL) {
                c->buffer = p;
            } else {
                dernier->suivant = p;
            }
            dernier = p;
        }
        printf("[RESTORATION] Etat du capteur restauré depuis save.bin\n");
    }

    fclose(f);
}

int main() {
    srand(time(NULL));

    Capteur c;
    restaurer_etat(&c);   // restaure l'état précédent si existant

    printf("=================================\n");
    printf("       ECO-SENSING SYSTEM\n");
    printf("=================================\n");
    printf("Energie initiale : %.2f J\n", c.energie);
    printf("Buffer max       : %d paquets\n\n", MAX_BUFFER);

    /* le programme ne s'arrete que si energie == 0 */
    while (c.energie > 0) {

        printf("---------------------------------\n");
        printf("[CAPTEUR] Nouvelle mesure\n");

        Paquet* p = creer_paquet(c.id_suivant++);
        if (p == NULL) {
            printf("Erreur allocation memoire\n");
            break;
        }

        afficher_paquet(p);

        ajouter_paquet(&c.buffer, p, &c.taille_buffer);

        /* Transmission avec condition */
        if (p->distance <= 20) {
            transmettre_paquet(&c.buffer, &c.taille_buffer, &c.energie);
        }

        // Consommation passive "circuit" (0.05 J par cycle)
        float consommation_passive = 0.05f;
        c.energie -= consommation_passive;
        if (c.energie < 0.0f) c.energie = 0.0f;

        printf("Energie restante : %.2f J\n", c.energie);

        // Journalisation dans log.txt
        FILE *log = fopen("log.txt", "a");
        if (log != NULL) {
            fprintf(log, "Temps: %ds | Batterie: %.2fJ | Paquets en attente: %d\n",
                    c.id_suivant - 1,
                    c.energie,
                    c.taille_buffer);
            fclose(log);
        } else {
            printf("Erreur ouverture log.txt\n");
        }

        /* Sauvegarde de l'état après chaque mesure */
        c.termine = 0;
        sauvegarder_etat(&c);

        sleep(1);   // pause de 1 seconde
    }

    printf("\nBatterie epuisee.\n");
    printf("Arret du systeme Eco-Sensing.\n");

    /* Marque que le programme s'est terminé normalement */
    c.termine = 1;
    sauvegarder_etat(&c);

    printf("Liberation de la memoire...\n");
    liberer_buffer(&c.buffer, &c.taille_buffer);

    printf("Nombre total de paquets transmis : %d\n", c.id_suivant - 1);
    printf("Programme termine correctement.\n");
    return 0;
}
