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



 int exitFlag=0;

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
    r->exit = &exitFlag;
    signal(SIGINT, signalHandler);
    //crear thread servidor 
    //// pthread_t threadserver;
    ////pthread_create(&threadserver,NULL,runServer,(void *)r);
  
    //iniciar terminal

    runTerminal(r, fitxerStock);
   
    for(int i=0;i<r->n_fils;i++){
        pthread_join(r->fils[i].thread_id,NULL);
    }
    for(int i=0;i<r->n_serverThreads;){
       // if(r->st[i]->thread_complete==1){
            
           // pthread_cancel(r->st[i]->thread_id);
             close(r->st[i]->client_fd);
            pthread_join(r->st[i]->thread_id, NULL);
          
            removeThreadServidor(r, i); 
        /*} else {
            i++; 
        }*/
    }
    close(r->fd_server);
    freeRegne(r);
    /*pid_t pid = fork();
        if (pid == 0) {
            pthread_kill(threadserver, SIGKILL);
            _exit(0);   
    }*/
    return 0;     
}