
#ifndef SERVER_H
#define SERVER_H
 
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "funcions_basiques.h"
int create_server_socket(int port);
int accept_client_connection(int server_fd);

void *runServer(void *portv);
#endif