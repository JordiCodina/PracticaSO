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

 
pthread_t threadserver;
char exitFlag=0;



void signalHandler() {
    exitFlag=1;
    signal(SIGINT, SIG_DFL);
    //raise(SIGINT);
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

    signal(SIGINT, signalHandler);
    //crear thread servidor 
   
    pthread_create(&threadserver,NULL,runServer,(void *)r);

    //iniciar terminal
    runTerminal(r, fitxerStock,&exitFlag);
    //hacer un kill
    //pthread_kill(threadserver, SIGTERM);

    if(exitFlag){
        pid_t pid = fork();
            if (pid == 0) {
                 pthread_kill(threadserver, SIGKILL);
                _exit(0);   
            }
       
    }else{
        pthread_join(threadserver,(void**)NULL);
    }
  
    return 0;     
}