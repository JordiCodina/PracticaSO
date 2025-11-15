
#ifndef SERVER_H
#define SERVER_H
 #define _GNU_SOURCE 
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include "funcions_basiques.h"
#include "client.h"
#include "config.h"
int create_server_socket(int port);
int accept_client_connection(int server_fd);

//Per pledge respond
void  respondPledge(int res);


void *runServer(void *aregne);
#endif