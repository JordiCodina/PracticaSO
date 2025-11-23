
#ifndef CONFIG_H
#define CONFIG_H


#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "funcions_basiques.h"

typedef struct
{
    char name[100];
    int amount;
    float weight;
} Producte;

typedef struct {

    char status;
    char *nomRegneViatge;

} Envoy;

typedef struct {

    char *nomDest;
    char *ipDest;
    int portDest;
    int esDirectamentConectat; // 1 -> sí, 0 -> no // es conegut no es el mateix que alianca , es per el cas de les ip amb *.*.*.* 0 de la configuracio inical.Llavors si es 0 representa  que si fem una aliança = 'S' haurem de substiruir aquest valors de ip i port, si no no cal
    char alianca; // "S" o "N" // si/no son aliats ,o "P" // s'ha fet una request

} Ruta;


typedef struct {

    pthread_t thread_id;
    int comanda;

} Fils;
typedef struct {

    char *nom;
    char *rutaArxius;
    char * ip;
    unsigned int port;

    Envoy *envoys;
    int n_envoys;

    Ruta *rutes;
    int n_rutes;

    int fd_server;
    int *exit;

    Fils *fils;//aixo potser s'hauria de posar a envoys pero de moment tenim nomes 1
    int n_fils;

    int envoyTemporalIsBusy;//si 1 esta ocupat , si 0 esta lliure //provisional per fase 2

} Regne;







//funcions d'alliberar memòria dinàmica

void freeEnvoy(Envoy e);

void freeRuta(Ruta r);

void freeRegne(Regne *r);

//funcions d'inicialitzar els strucs

void initializeEnvoy (Envoy *e);

void initializeRuta (Ruta *r);

void initializeRegne (Regne *r);

//funcions per copiar els structs 

void envoyCpy (Envoy *dest, Envoy *org);

void rutaCpy (Ruta *dest, Ruta *org);

void regneCpy (Regne *dest, Regne* org);

void cpyProducte (Producte *dest, Producte *org);

//funcions per extreure la informació dels fitxers
Regne *extractInfo (const char *nomFitxer);

Producte *extreureProductes(int fProductes, int *n_productes);


int comprovarRegneExisteix(Regne *r, char * nomRegneRuta);

#endif