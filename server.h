
#ifndef SERVER_H
#define SERVER_H
 #define _GNU_SOURCE 
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include "funcions_basiques.h"
#include "client.h"
#include "config.h"
#include "producte.h"


#define ERROR_MALLOC "Error: malloc falló\n"
#define ERROR_REALLOC "Error: realloc falló\n"
#define WAITING_CONNECTION "Esperando conexiones...\n"
#define CLIENT_CONNECTED "¡Cliente conectado!\n"
#define USAGESERVER "Uso: ./server <palabra> <max_intentos> <puerto>\n"
#define ERROR_SOCKET "Error en la creación del socket"
#define ERROR_BIND "Error en el bind del socket\n"
#define ERROR_LISTEN "Error en el listen del socket\n"
#define ERROR_ACCEPT "Error en el accept de la conexión\n"
#define MAX_TRIES "Error: max_intentos debe ser mayor que 0 y al menos 3 más que la longitud de la palabra\n"

#define BUFFER_SIZE 256

int create_server_socket(int port);
int accept_client_connection(int server_fd);

//Per pledge respond
void  respondPledge(int res);

void *runServer(void *aregne);

void closeServer(Regne *r);
#endif