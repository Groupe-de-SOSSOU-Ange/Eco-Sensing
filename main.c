#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "sen.h"

/* Structure du Capteur */
typedef struct capteur {
    float energie;
    int id_suivant;
    Paquet* buffer;
    int taille_buffer;
    int termine;
    int x, y; // initialisation des  coordonnées de l'emplacement du capteur
} Capteur;

/* Sauvegarde et restauration */
void sauvegarder_etat(Capteur *c) {
    FILE *f = fopen("save.bin", "wb");
    if (!f) return;

    fwrite(&c->energie, sizeof(float), 1, f);
    fwrite(&c->id_suivant, sizeof(int), 1, f);
    fwrite(&c->taille_buffer, sizeof(int), 1, f);
    fwrite(&c->termine, sizeof(int), 1, f);
    fwrite(&c->x, sizeof(int), 1, f);
    fwrite(&c->y, sizeof(int), 1, f);

    Paquet* cur = c->buffer;
    while (cur) {
        fwrite(cur, sizeof(Paquet), 1, f);
        cur = cur->suivant;
    }
    fclose(f);
}

void restaurer_etat(Capteur *c) {
    FILE *f = fopen("save.bin", "rb");
    if (!f) {
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
    fread(&c->x, sizeof(int), 1, f);
    fread(&c->y, sizeof(int), 1, f);

    if (c->termine == 1) {
        c->energie = 5.0f;
        c->id_suivant = 1;
        c->taille_buffer = 0;
        c->buffer = NULL;
        c->termine = 0;
        printf("[INFO] Redemarrage a zero avec batterie 5 J.\n");
    } else {
        Paquet* dernier = NULL;
        for (int i = 0; i < c->taille_buffer; i++) {
            Paquet* p = malloc(sizeof(Paquet));
            fread(p, sizeof(Paquet), 1, f);
            p->suivant = NULL;
            if (!dernier) c->buffer = p;
            else dernier->suivant = p;
            dernier = p;
        }
        printf("[RESTORATION] etat du capteur restaurer.\n");
    }
    fclose(f);
}

int main() {
    srand(time(NULL));

    Capteur c;
    restaurer_etat(&c);

    if (c.id_suivant == 1) { // saisie des coordonnées seulement s'il y a une nouvelle simulation
        printf("Entrez coordonnee x : ");
        scanf("%d", &c.x);
        printf("Entrez coordonnee y : ");
        scanf("%d", &c.y);
    }

    printf("=================================\n");
    printf("       ECO-SENSING SYSTEM\n");
    printf("=================================\n");
    printf("Energie initiale : %.2f J\n", c.energie);
    printf("Buffer max       : %d paquets\n\n", MAX_BUFFER);

    while (c.energie > 0) {
        printf("---------------------------------\n");
        printf("[CAPTEUR] Nouvelle mesure\n");

        // calcul distance depuis origine (0,0) → coordonnées choisies par l'utilisateur
        int distance = (int)sqrt(c.x * c.x + c.y * c.y);

        Paquet* p = creer_paquet(c.id_suivant++, distance);
        if (!p) {
            printf("Erreur allocation memoire\n");
            break;
        }

        afficher_paquet(p);

        ajouter_paquet(&c.buffer, p, &c.taille_buffer);

        transmettre_paquet(&c.buffer, &c.taille_buffer, &c.energie);

        // consommation passive
        float consommation_passive = 0.05f;
        c.energie -= consommation_passive;
        if (c.energie < 0.0f) c.energie = 0.0f;

        printf("Energie restante : %.2f J\n", c.energie);

        // log
        FILE *log = fopen("log.txt", "a");
        if (log) {
            fprintf(log, "Temps: %ds | Batterie: %.2fJ | Paquets en attente: %d\n",
                    c.id_suivant - 1, c.energie, c.taille_buffer);
            fclose(log);
        }

        c.termine = 0;
        sauvegarder_etat(&c);
        sleep(1);
    }

    printf("\nBatterie epuisee.\n");
    printf("Arret du systeme Eco-Sensing.\n");

    c.termine = 1;
    sauvegarder_etat(&c);

    printf("Liberation de la memoire...\n");
    liberer_buffer(&c.buffer, &c.taille_buffer);

    printf("Nombre total de paquets transmis : %d\n", c.id_suivant - 1);
    printf("Programme termine correctement.\n");

    return 0;
}
