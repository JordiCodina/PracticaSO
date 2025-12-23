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
    key_t shm_key = ftok(r->nom, 1);
    int shmid = shmget(shm_key, sizeof(shared_data), IPC_CREAT | 0666);
    shared_data *shm = (shared_data *)shmat(shmid, NULL, 0);


    printf("Maester of %s starting with PID %d , nenvoys %d\n", r->nom, getpid(), r->n_envoys);
    
    for (int i = 0; i < r->n_envoys; i++) {

        int pipePareFill[2];
        int pipeFillPare[2];
        if (pipe(pipePareFill) == -1) {
            perror("pipe pare->fill");
            return -1; 
        }

        if (pipe(pipeFillPare) == -1) {
            perror("pipe fill->pare");
            return -1;
        }

        pid_t pid = fork();
        r->envoys[i].pid = pid;
        if (pid < 0) {
            perror("fork");
            return -1;
        }

        if (pid == 0) {
            close(pipePareFill[1]); 
            close(pipeFillPare[0]);

            runenvoy(shm_key,pipePareFill[0], pipeFillPare[1], r); 

            _exit(0);
        }

        close(pipePareFill[0]); 
        close(pipeFillPare[1]);

        r->envoys[i].pipeWrite = pipePareFill[1]; // escriure al envoy
        r->envoys[i].pipeRead  = pipeFillPare[0]; // llegir del envoy
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

    for(int i=0;i<r->n_fils;i++){
        pthread_join(r->fils[i].thread_id,NULL);
    }
    for(int i=0;i<r->n_serverThreads;){
        close(r->st[i]->client_fd);
        pthread_join(r->st[i]->thread_id, NULL);
        removeThreadServidor(r, i); 
    }

    close(r->fd_server);
    freeRegne(r);
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl IPC_RMID");
    }

    return 0;     
}