
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




//comandes
void listProductsMaester(char *nomFitxer);
void startTrade(Regne * regneActual, char* comand);

#endif
