#ifndef _REGNE_H_
#define _REGNE_H_

#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#include "funcions_basiques.h"
#include "config.h"
#include "client.h"
#include "server.h"
#include "semaphore.h"




#define FILENAME_SIGIL_SIZE 100
#define DATA_SIZE 275
#define COM_LEN 256

typedef struct Thread_pledge {
    Regne *r;
    char **paraules;
    int n_paraules;
    int indexEnvoy;
} Thread_pledge;

int comprovarRutaRegneValid(char *str,int n_rutes , Ruta *rutes);

// Comanda LIST REALMS

void listRealms(Regne *r);

// Comanda PLEDGE
//void sendEnvoy(Regne *r, char *casa);
//void pledge (Regne *r, char comanda [100]);
//void pledge (Regne *r,char **paraules);
void* pledge (void *arg);
// Comanda PLEDGE RESPOND
void pledgeRespond ( Regne *r,char** paraules,int n_paraules);
// Comanda PLEDGE STATUS 
void pledgeStatus (Regne *r);
// Comanda ENVOY STATUS
void envoyStatus (Regne *r);




void notificarDesconnexioAliances(Regne *r);
void closeServer(Regne *r);

char *escoltarTot(fd_set *master_set, int *max_fd, Regne *r);

void borrarDirectoris(Regne *r);
#endif