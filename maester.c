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


#define BUFFER_SIZE 256

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

    int port = r->port;
        pthread_t threads[5];
        pthread_create(&threads[0],NULL,runServer,(void *)&port);

        /*
        for (int i = 1; i < 3; i++){
            pthread_join(threads[i-1],**(void));
        }*/
    if(r->port==8110){
        int portn=8111;
        runClient(portn, r->ip); 
    }
    runTerminal(r, fitxerStock);
    pthread_join(threads[0],(void**)NULL);
    return 0;    
}