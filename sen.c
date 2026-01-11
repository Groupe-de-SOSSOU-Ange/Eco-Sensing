#include <stdio.h>
#include <stdlib.h>
#include "sen.h"

/* Création d'un paquet */
Paquet* creer_paquet(int id) {
    Paquet* p = malloc(sizeof(Paquet));
    if (p == NULL) return NULL;

    p->id = id;
    p->temperature = 20 + rand() % 15;
    p->humidite = 40 + rand() % 40;

    // Transmission aléatoire
    if (rand() % 2 == 0) {
        p->distance = 10;  // paquet transmis
    } else {
        p->distance = 50;  // paquet reste dans le buffer
    }

    p->suivant = NULL;
    return p;
}



/* Affichage */
void afficher_paquet(Paquet* p) {
    printf("Paquet ID : %d\n", p->id);
    printf("Temperature : %.1f Celsus\n", p->temperature);
    printf("Humidite    : %.1f %%\n", p->humidite);
    printf("Distance    : %d m\n", p->distance);
}

/* Suppression FIFO */
void supprimer_plus_ancien(Paquet** buffer, int* taille_buffer) {
    if (*buffer == NULL) return;

    Paquet* temp = *buffer;
    *buffer = temp->suivant;

    printf("[ALERTE] Buffer plein -> suppression du paquet ID %d\n", temp->id);
    free(temp);
    (*taille_buffer)--;
}

/* Ajout dans le buffer */
void ajouter_paquet(Paquet** buffer, Paquet* p, int* taille_buffer) {
    if (*taille_buffer >= MAX_BUFFER) {
        supprimer_plus_ancien(buffer, taille_buffer);
    }

    if (*buffer == NULL) {
        *buffer = p;
    } else {
        Paquet* courant = *buffer;
        while (courant->suivant != NULL)
            courant = courant->suivant;
        courant->suivant = p;
    }

    (*taille_buffer)++;
    printf("Paquet ID %d ajoute au buffer\n", p->id);
}

/* Transmission = consommation d'énergie selon la formule Etx = Eelec + Eamp*d^2 */
void transmettre_paquet(Paquet** buffer, int* taille_buffer, int* energie) {
    if (*buffer == NULL) return;

    Paquet* p = *buffer;
    *buffer = p->suivant;

    printf("[TRANSMISSION] Paquet ID %d transmis\n", p->id);

    free(p);
    (*taille_buffer)--;

    /* formule de decharge */
    float Eelec = 0.05;         // énergie électronique en J
    float Eamp  = 0.01;         // énergie amplificateur en J/m²
    float Etx   = Eelec + Eamp * (p->distance * p->distance);

    *energie -= (int)Etx;
    if (*energie < 0) *energie = 0;
}

/* Libération finale */
void liberer_buffer(Paquet** buffer, int* taille_buffer) {
    while (*buffer != NULL) {
        Paquet* temp = *buffer;
        *buffer = temp->suivant;
        printf("Liberation du paquet ID %d\n", temp->id);
        free(temp);
    }
    *taille_buffer = 0;
}
