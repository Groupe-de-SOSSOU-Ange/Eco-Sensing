#include <stdio.h>
#include <stdlib.h>
#include "sen.h"

/* Création d'un paquet */
Paquet* creer_paquet(int id, int distance_fix) {
    Paquet* p = malloc(sizeof(Paquet));
    if (!p) return NULL;

    p->id = id;
    p->temperature = 20 + rand() % 15;
    p->humidite = 40 + rand() % 40;
    p->distance = distance_fix; // distance fixe pour tout le programme
    p->suivant = NULL;

    return p;
}

/* Affichage */
void afficher_paquet(Paquet* p) {
    printf("Paquet ID : %d\n", p->id);
    printf("Temperature : %.1f C\n", p->temperature);
    printf("Humidite    : %.1f %%\n", p->humidite);
    printf("Distance    : %d m\n", p->distance);
}

/* Suppression du plus ancien paquet */
void supprimer_plus_ancien(Paquet** buffer, int* taille_buffer) {
    if (!*buffer) return;

    Paquet* temp = *buffer;
    *buffer = temp->suivant;

    printf("[ALERTE] Buffer plein -> suppression du paquet ID %d\n", temp->id);
    free(temp);
    (*taille_buffer)--;
}

/* Ajout de paquet dans le buffer */
void ajouter_paquet(Paquet** buffer, Paquet* p, int* taille_buffer) {
    if (*taille_buffer >= MAX_BUFFER) {
        supprimer_plus_ancien(buffer, taille_buffer);
    }

    if (!*buffer) {
        *buffer = p;
    } else {
        Paquet* courant = *buffer;
        while (courant->suivant) courant = courant->suivant;
        courant->suivant = p;
    }
    (*taille_buffer)++;
    printf("Paquet ID %d ajoute au buffer\n", p->id);
}

/* une transmission aléatoire = consommation d'énergie */
void transmettre_paquet(Paquet** buffer, int* taille_buffer, float* energie) {
    if (!*buffer) return;

    if (rand() % 2 == 0) { // transmission aléatoire 50%
        Paquet* p = *buffer;
        *buffer = p->suivant;

        printf("[TRANSMISSION] Paquet ID %d transmis\n", p->id);

        float Eelec = 0.05f;
        float Eamp = 0.01f;
        float Etx = Eelec + Eamp * (p->distance * p->distance);

        *energie -= Etx;
        if (*energie < 0.0f) *energie = 0.0f;

        free(p);
        (*taille_buffer)--;
    }
}

/* Libération finale */
void liberer_buffer(Paquet** buffer, int* taille_buffer) {
    while (*buffer) {
        Paquet* temp = *buffer;
        *buffer = temp->suivant;
        printf("Liberation du paquet ID %d\n", temp->id);
        free(temp);
    }
    *taille_buffer = 0;
}
