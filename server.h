
#ifndef SERVER_H
#define SERVER_H
 
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int create_server_socket(int port);
int accept_client_connection(int server_fd);

#endif