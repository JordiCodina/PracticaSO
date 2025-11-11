#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "client.h"


#define WON_MESSAGE "¡Has ganado!\n"
#define LOST_MESSAGE "¡Juego terminado! Has perdido.\n"
#define INVALID_ADDRESS "Dirección inválida\n"
#define CONNECTION_FAILED "Conexión fallida\n"
#define SOCKET_CREATION_FAILED "Falló la creación del socket\n"
#define ASK_LETTER "Introduce una letra: "
#define INVALID_INPUT "Entrada inválida. Solo se puede introducir una letra\n"
#define USAGE "Uso: ./client <ip> <puerto>\n"


int create_client_socket(const char *ip, int port) {
    int client_sock;
    struct sockaddr_in serv_addr;
    
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0) {
        printString(1, SOCKET_CREATION_FAILED);
        return -1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        printString(1, INVALID_ADDRESS);
        close(client_sock);
        return -1;
    }
    
    if (connect(client_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printString(1, CONNECTION_FAILED);
        close(client_sock);
        return -1;
    }
    
    return client_sock;
}


int runClient(int port, char *ip) {
   // int port = *(int *)portv;
        int sock = create_client_socket(ip, port);
        if (sock < 0) {
    
            printString(1,"Error creating the socket\n");
            return 0;
        }
         printString(1,"Socket creat i connectat!\n");
        char msg = 'A';
        if (write(sock, &msg, 1) < 0) {
            printString(1,"Error enviant el missatge\n");
            close(sock);
            return 1;
        }

    printString(1,"Missatge enviat correctament!\n");

    close(sock);
    return 0;

}