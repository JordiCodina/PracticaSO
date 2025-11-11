
#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "funcions_basiques.h"
#include "terminal.h"
#include "server.h"
#include "client.h"

 
#define BUFFER_SIZE 1<<8
#define printF(x) write(1, x, strlen(x))

int main() {

    int port = 8111;
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

    printF("Maester: Client connected\n");

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