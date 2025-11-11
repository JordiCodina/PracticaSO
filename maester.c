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


#define BUFFER_SIZE 1<<8
#define printF(x) write(1, x, strlen(x))





int runServer(void * portv){
    int port = *(int *)portv;
    int server_fd = create_server_socket(port);
    if (server_fd < 0) {
        printF("Error creant el servidor\n");
        return 1;
    }

    printF("Maester: Waiting for connections...\n");

    int new_socket = accept_client_connection(server_fd);
    if (new_socket < 0) {
        close(server_fd);
        return 1;
    }


    printString(1,"Maester: Client connected\n");

    char buffer[BUFFER_SIZE];
    memset(buffer, '\0', sizeof(buffer));

    ssize_t valread = read(new_socket, buffer, BUFFER_SIZE - 1);
    if (valread < 0) {
        printF("Maester: Error reading from socket\n");
        close(new_socket);
        close(server_fd);
        return 1;
    }

    buffer[valread] = '\0';
    printF("Maester: Received message: ");
    printF(buffer);
    printF("\n");

    close(new_socket);
    close(server_fd);
    return 0;
}   

int runClient(int port){
   // int port = *(int *)portv;
            static char *ip = "127.0.0.1"; 


        int sock = create_client_socket(ip, port);
        printString(1,"Socket creat\n");
        if (sock < 0) {
    
            printString(1,"Error creating the fork\n");
            return 0;
        }
         printString(1,"Socket creat i connectat!\n");
        char msg = 'A';
        if (write(sock, &msg, 1) < 0) {
            printF("Error enviant el missatge\n");
            close(sock);
            return 1;
        }

    printF("Missatge enviat correctament!\n");

    close(sock);
    return 0;

}

int main (int argc, char *argv[]){
    if (argc != 3) {
        printString(1,"Incorrect arguments : Maester <realm_name> <stock_file>\n");
        return 1;
    }

    Regne *r = extractInfo (argv[1]);
    char *fitxerStock = argv[2];
    if (r == NULL) return;

    int port = r->port;
    printf("%d\n", port);
        pthread_t threads[5];
        pthread_create(&threads[0],NULL,runServer,(void *)&port);

        /*
        for (int i = 1; i < 3; i++){
            pthread_join(threads[i-1],**(void));
        }*/
    if(r->port==8110){
        int portn=8111;
        runClient(portn); 
    }
    runTerminal(r, fitxerStock);
    pthread_join(threads[0],(void**)NULL);
    return 0;    
}