
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

#define FILENAME_STOCK_SIZE 100

//comandes
void listProductsMaester(char *nomFitxer, int opcio,char * nomAliat) ;
void listProductsAliat(Regne *r, char *nomAliat);
void startTrade(Regne *r, char *comand, fd_set *master_set, int *max_fd) ;


char* crearRutaComanda(char * rutaArxius, char * nomMaester, char * nomRegneDest);
int crearFitxerComandes(char *nomFitxer);

void actualitzarProductes(char *nomFitxer, Producte *productes, int n_productes);
int treureDelStock(Producte *productes, int *n_productes, const char *nom, int quantitat) ;
void afegirAlStock(Producte *productes, int *n_productes,const char *nom, int quantitat, float pes) ;

#endif
