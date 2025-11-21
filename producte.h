
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

#define FILENAME_STOCK_SIZE 100



//comandes
void listProductsMaester(char *nomFitxer);
void startTrade(Regne * regneActual, char* comand);

#endif
