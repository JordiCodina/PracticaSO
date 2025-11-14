
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
    char alianca;

} Ruta;

typedef struct {

    char *nom;
    char *rutaArxius;
    char * ip;
    unsigned int port;

    Envoy *envoys;
    int n_envoys;

    Ruta *rutes;
    int n_rutes;

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

#endif