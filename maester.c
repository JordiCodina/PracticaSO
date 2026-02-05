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
#include <signal.h>





#include "funcions_basiques.h"
#include "terminal.h"
#include "server.h"
#include "client.h"
#include "envoy_process.h"
#include "semaphore.h"




int exitFlag = 0;

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
    
    SEM_constructor(&r->mutexLectura);  
    SEM_init(&r->mutexLectura, 1);
    for (int i = 0; i < r->n_envoys; i++) {

        int pipePareFill[2];
        int pipeFillPare[2];
        if (pipe(pipePareFill) == -1) {
            return -1; 
        }

        if (pipe(pipeFillPare) == -1) {
            return -1;
        }

        pid_t pid = fork();
        r->envoys[i].pid = pid;
        if (pid < 0) {
            printString(1, "Error forking envoy process\n");
            return -1;
        }

        if (pid == 0) {
            for (int j = 0; j < i; j++) {
                close(r->envoys[j].pipeRead);
                close(r->envoys[j].pipeWrite);
            }
            close(pipePareFill[1]); 
            close(pipeFillPare[0]);

            runenvoy(pipePareFill[0], pipeFillPare[1], r); 

            freeRegne(r);

            _exit(0);
        }

        close(pipePareFill[0]); 
        close(pipeFillPare[1]);

        r->envoys[i].pipeWrite = pipePareFill[1]; 
        r->envoys[i].pipeRead  = pipeFillPare[0]; 
    }
    
    
    char *fitxerStock = argv[2];
    if (r == NULL) { 
        printString(1,"Error extracting realm info\n");
        return 0; 
    } 
    signal(SIGINT, signalHandler);
    r->exit = &exitFlag;
    
    //borrar directori antic si existeix  
    borrarDirectori(r->rutaArxius);

  
    

    //iniciar terminal
    runTerminal(r, fitxerStock);

    for(int i=0;i<r->n_serverThreads;){
        close(r->st[i]->client_fd);
        pthread_join(r->st[i]->thread_id, NULL);
        removeThreadServidor(r, i); 
    }

    for (int j = 0; j < r->n_envoys; j++) {//tancar pipes i esperar processos envoy
            close(r->envoys[j].pipeRead);
            close(r->envoys[j].pipeWrite);
            waitpid(r->envoys[j].pid, NULL, 0);
    }
    SEM_destructor(&r->mutexLectura);


    close(r->fd_server);
    freeRegne(r);

    return 0;     
}