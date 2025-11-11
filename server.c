

#include "server.h"

#define ERROR_MALLOC "Error: malloc falló\n"
#define ERROR_REALLOC "Error: realloc falló\n"
#define WAITING_CONNECTION "Esperando conexiones...\n"
#define CLIENT_CONNECTED "¡Cliente conectado!\n"
#define USAGE "Uso: ./server <palabra> <max_intentos> <puerto>\n"
#define ERROR_SOCKET "Error en la creación del socket"
#define ERROR_BIND "Error en el bind del socket\n"
#define ERROR_LISTEN "Error en el listen del socket\n"
#define ERROR_ACCEPT "Error en el accept de la conexión\n"
#define MAX_TRIES "Error: max_intentos debe ser mayor que 0 y al menos 3 más que la longitud de la palabra\n"
#define BUFFER_SIZE 256
int create_server_socket(int port) {
    int server_fd;
    struct sockaddr_in address;
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        printString(1,ERROR_SOCKET);
        return -1;
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        printString(1,ERROR_BIND);
        close(server_fd);
        return -1;
    }
    
    if (listen(server_fd, 3) < 0) {
        printString(1,ERROR_LISTEN);
        close(server_fd);
        return -1;
    }
    
    return server_fd;
}
int accept_client_connection(int server_fd) {
    int new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if (new_socket < 0) {
        printString(1,ERROR_ACCEPT);
        return -1;
    }
    
    return new_socket;
}
void* runServer(void * portv){
    int port = *(int *)portv;
    int server_fd = create_server_socket(port);
    if (server_fd < 0) {
        printString(1,"Error creant el servidor\n");
        return (void*)1;
    }

    printString(1,"Maester: Waiting for connections...\n");
    int new_socket = accept_client_connection(server_fd);
    if (new_socket < 0) {
        close(server_fd);
        return (void*)1;
    }


    printString(1,"Maester: Client connected\n");

    char buffer[BUFFER_SIZE];
    memset(buffer, '\0', sizeof(buffer));

    ssize_t valread = read(new_socket, buffer, BUFFER_SIZE - 1);
    if (valread < 0) {
        printString(1,"Maester: Error reading from socket\n");
        close(new_socket);
        close(server_fd);
        return (void*)1;
    }

    buffer[valread] = '\0';
    printString(1,"Maester: Received message: ");
    printString(1,buffer);
    printString(1,"\n");

    close(new_socket);
    close(server_fd);
    return (void*)0;
}   
