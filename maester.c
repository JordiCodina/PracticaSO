#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#include "funcions_basiques.h"
#include "terminal.h"
#include "server.h"
#include "client.h"


//crear conf harcoded
ClientConf *crearEnviamentArtificial(Regne *r) {
    Trama *trama = crearTrama(r, 0x01, "TheVale", "Loocoooo");
    ClientConf *clientConf = trobarSeguentSalt(r, trama); //pasar el regne per el routing i trama 
    printf("Creating artificial ClientConf to %s:%d\n", clientConf->ipDest, clientConf->portDest);
    if (!clientConf) {
        printString(1, "Error obtenint ClientConf\n");
        return NULL;
    }
    return clientConf;
}

int main (int argc, char *argv[]){
    if (argc != 3) {
        printString(1,"Incorrect arguments : Maester <realm_name> <stock_file>\n");
        return 1;
    }
    Regne *r = extractInfo (argv[1]);
    char *fitxerStock = argv[2];
    if (r == NULL) {
        printString(1,"Error extracting realm info\n");
        return 0;
    }
    //crear thread servidor 
    pthread_t threads[5];
    pthread_create(&threads[0],NULL,runServer,(void *)r);
    //////////////////////////////////////// 
    if(strcmp(r->nom,"Dragonstone")==0){//per proves, comença a Dragonstone l'enviament artifical
        ClientConf *c = crearEnviamentArtificial(r);//per proves
        pthread_create(&threads[1], NULL, runClient, (void*)c);
    }
    ////////////////////////////////////////
    runTerminal(r, fitxerStock);
    pthread_join(threads[0],(void**)NULL);
    return 0;    
}