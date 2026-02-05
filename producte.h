
#ifndef PRODUCTES_H
#define PRODUCTES_H

#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include "funcions_basiques.h"
#include "config.h"
#include "client.h"
#include "regne.h"
#include "semaphore.h"

#define FILENAME_STOCK_SIZE 100

//comandes
void listProductsMaester(char *nomFitxer, int opcio,char * nomAliat) ;

int comprovarlistproducts(Regne * r, char *nomAliat);
int listProductsAliat(Regne *r, char *nomAliat,/*int* exitFlag,*/unsigned int portDestinat,char *ipDestinat) ;
int sendTrade( char *nomFitxer, char *nomRegneDest, Regne *r,unsigned int portDestinat,char *ipDestinat);//send comanda

void startTrade(Regne *r, char *comand, fd_set *master_set, int *max_fd) ;//començar start trade


char* crearRutaComanda(char * rutaArxius, char * nomMaester, char * nomRegneDest);
int crearFitxerComandes(char *nomFitxer);

void actualitzarProductes(char *nomFitxer, Producte *productes, int n_productes);
int treureDelStock(Producte *productes, int *n_productes, const char *nom, int quantitat) ;
void afegirAlStock(Producte *productes, int *n_productes,const char *nom, int quantitat, float pes) ;



#endif
