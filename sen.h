#ifndef SENSOR_H
#define SENSOR_H

#define MAX_BUFFER 5

typedef struct paquet {
    int id;
    float temperature;
    float humidite;
    int distance;
    struct paquet* suivant;
} Paquet;

/* Capteur */
Paquet* creer_paquet(int id, int distance_fix);
void afficher_paquet(Paquet* p);

/* Buffer */
void ajouter_paquet(Paquet** buffer, Paquet* p, int* taille_buffer);
void supprimer_plus_ancien(Paquet** buffer, int* taille_buffer);

/* Transmission */
void transmettre_paquet(Paquet** buffer, int* taille_buffer, float* energie);

/* Nettoyage */
void liberer_buffer(Paquet** buffer, int* taille_buffer);

#endif
