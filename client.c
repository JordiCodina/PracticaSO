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



#define BUFFER_SIZE 256
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

Trama * crearTrama(Regne * r,char tipus,char *nom_desti, char *data) {
    Trama * t = malloc (sizeof(Trama));
    t->tipus = tipus;
    snprintf(t->nom_origen, sizeof(t->nom_origen), "%s:%d", r->ip, r->port);
    snprintf(t->nom_desti, sizeof(t->nom_desti), "%s", nom_desti);
    snprintf(t->data, sizeof(t->data), "%s", data);
    t->dataLength = strlen(t->data);
    t->checksum = 2; // calcularChecksum(); 
    return t;
}

//aquesta funcio busca la seguent ruta (tant si esta directament conectada com si no i llavors s'ha de fer salt) 
ClientConf *trobarSeguentSalt(Regne *r, Trama *t) {//si troba ip amb * , agafa default
    int trobat = -1;
    for (int i = 0; i < r->n_rutes; i++) {
        if (strcmp(r->rutes[i].nomDest, t->nom_desti) == 0) {
            trobat = i;
            break;
        }
    }
    int idx = 0; 
    if (trobat >= 0 && !teCaracter(r->rutes[trobat].ipDest, '*')) {
        idx = trobat;
    }

    ClientConf *conf = malloc(sizeof(ClientConf));
    if (!conf) {
        printString(1, "Error allocating memory for ClientConf\n");
        return NULL;
    }
    conf->portDest = r->rutes[idx].portDest;
    conf->ipDest   = r->rutes[idx].ipDest;   
    conf->nomDest  = r->rutes[idx].nomDest;  
    conf->trama    = *t;                    
    printf("Trama destinada a %s (%s:%d)\n", conf->nomDest, conf->ipDest, conf->portDest);
    return conf;
}


void *runClient(void *arg)
{
    ClientConf *conf = (ClientConf *)arg;

    int client_sock = create_client_socket(conf->ipDest, conf->portDest);
    if (client_sock < 0) {
        printString(1, "Error creant el socket\n");
        
        free(conf);
        return NULL;
    }
    printString(1, "Enviant trama...\n");
    write(client_sock, &conf->trama, sizeof(Trama));
    close(client_sock);
    free(conf);
    return NULL;
}

