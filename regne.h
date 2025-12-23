#ifndef _REGNE_H_
#define _REGNE_H_

#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>


#include "funcions_basiques.h"
#include "config.h"
#include "client.h"
#include "server.h"
#include "semaphore.h"




#define FILENAME_SIGIL_SIZE 100
#define DATA_SIZE 275
#define COM_LEN 256

typedef struct _Thread_pledge {
    //id
    Regne *r;
    char **paraules;
    int n_paraules;
    int indexEnvoy;
} Thread_pledge;



typedef struct _ForkRegneWritesEnvoy {//struc que li enviem al envoy
    int comanda;//0 = pledge , 1 = list products , 2 = send products, 3 = ackFitxer , 4 = ackMD5SUM , 5= pledge respond
    char regneDest[50]; //per pledge ,list products , send products
    char  rutaSegell[30]; //per pledge
    int indexEnvoy; //per identificar l'envoy
    int ackStatus; // 1 = OK , 2 = KO ,  (per pledge respond , 1= accept , 2 = reject)
    int statusPledgeRespond; // 1 = accept , 2 = reject (per pledge respond)
    char ipDest[16];//pledge respond 
    int portDest; //pledge respond
} ForkRegneWritesEnvoy;

typedef struct _EnvoyWritesRegne {

    int comanda;//1 pledge , 2listproducts , 3 sendproducts 
    int statusPledge;// 1 = accept , 2 = reject , 3 = timeout
    int indexEnvoy;
    int indexRuta;
    int portDest;
    char ipDest[16];

    
} EnvoyWritesRegne;




int comprovarRutaRegneValid(char *str,int n_rutes , Ruta *rutes);

// Comanda LIST REALMS

void listRealms(Regne *r);

// Comanda PLEDGE
//void sendEnvoy(Regne *r, char *casa);
//void pledge (Regne *r, char comanda [100]);
//void pledge (Regne *r,char **paraules);
void* pledge (void *arg);
int pledgeComprovar(Regne *r, char **paraules, int n_paraules);
void pledgeEnvoyTramaInicial(char filenameSigil[30] , char nomDesti[50], Regne *r, int indexRuta);
void pledgeEnvoyEnviarSigil(char filenameSigil[30] , char nomDesti[50], Regne *r, int indexRuta,int *exitflag);
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