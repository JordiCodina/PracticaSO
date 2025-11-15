#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "client.h"



#define WON_MESSAGE "¡Has ganado!\n"
#define LOST_MESSAGE "¡Juego terminado! Has perdido.\n"
#define INVALID_ADDRESS "Dirección inválida\n"
#define CONNECTION_FAILED "Conexión fallida \n"
#define SOCKET_CREATION_FAILED "Falló la creación del socket\n"
#define ASK_LETTER "Introduce una letra: "
#define INVALID_INPUT "Entrada inválida. Solo se puede introducir una letra\n"
#define USAGE "Uso: ./client <ip> <puerto>\n"



#define BUFFER_SIZE 256

void serializeTrama(Trama *t, unsigned char *buffer) {
    int offset = 0;

    memcpy(buffer + offset, &t->tipus, 1);
    offset += 1;

    memcpy(buffer + offset, t->nom_origen, 20);
    offset += 20;

    memcpy(buffer + offset, t->nom_desti, 20);
    offset += 20;

    memcpy(buffer + offset, &t->dataLength, 2);
    offset += 2;

    memcpy(buffer + offset, t->data, 275);
    offset += 275;

    memcpy(buffer + offset, &t->checksum, 2);
    offset += 2;

    // opcional: comprova offset == 320
}
void deserializeTrama(unsigned char *buffer, Trama *t) {
    int offset = 0;

    memcpy(&t->tipus, buffer + offset, 1);
    offset += 1;

    memcpy(t->nom_origen, buffer + offset, 20);
    offset += 20;

    memcpy(t->nom_desti, buffer + offset, 20);
    offset += 20;

    memcpy(&t->dataLength, buffer + offset, 2);
    offset += 2;

    memcpy(t->data, buffer + offset, 275);
    offset += 275;

    memcpy(&t->checksum, buffer + offset, 2);
    offset += 2;
}










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

 void printMissHopping(ClientConf *conf, char *nomOrg, char *nomDest) {
    char *miss;
    asprintf(&miss,">>> Recived hop with ip:port -> %s to realm name -> %s\n Found route: %s -> %s:%d \n Forwarding PLEDGE request\n", nomOrg, nomDest, conf->nomDest, conf->ipDest,conf->portDest);
    printString(1, miss);
    free(miss);

 }

unsigned short calcularChecksum(Trama *t) {
    unsigned long suma = 0;  
    unsigned char *ptr = (unsigned char *)t;
    for (size_t i = 0; i < sizeof(Trama); i++) {
        if (ptr + i >= (unsigned char *)&t->checksum && ptr + i <  (unsigned char *)&t->checksum + sizeof(t->checksum)) {
            continue;
        }
        suma += ptr[i];
    }
    return (unsigned short)(suma % 65536);
}
Trama * crearTrama(char tipus,char *ip,int port,char *nom_desti,unsigned short dataLength,char *data) {
    Trama * t = malloc (sizeof(Trama));
    t->tipus = tipus;
    
    snprintf(t->nom_origen, sizeof(t->nom_origen), "%s:%d", ip, port);
    snprintf(t->nom_desti, sizeof(t->nom_desti), "%s", nom_desti);
    snprintf(t->data, sizeof(t->data), "%s", data);
    if(ip==NULL && port==0){//aixo es per les trames de errors o comprovacions com NACK o MD5SUM
       t->nom_origen[0]='\0';
       t->nom_desti[0]='\0';
    }
     if(tipus==0){
        t->checksum=0;
        t->dataLength=0;
        return t;
    }
    t->dataLength = dataLength;
    t->checksum = calcularChecksum(t); 
    return t;
}

//aquesta funcio busca la seguent ruta (tant si esta directament conectada com si no i llavors s'ha de fer salt) 
ClientConf *trobarSeguentMaester(Regne *r, Trama *t) {//si troba ip amb * , agafa default
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
    conf->trama = t;                    
    return conf;
}


void enviarTramaDirecte(Trama *t,int portDest,char *ipDest, char * nomDest){ 
    ClientConf *conf = malloc(sizeof(ClientConf));
    if (!conf) {
        printString(1, "Error allocating memory for ClientConf\n");
        return;
    }
    conf->portDest = portDest;
    conf->ipDest   = ipDest;   
    conf->nomDest  = nomDest;  
    conf->trama    = t;                    
    pthread_t th;
    pthread_create(&th, NULL, runClient, conf);
    pthread_join(th, NULL);
}

 void enviarTrama(Trama *t, Regne *r, int esHopping) {//esHopping = 1 aixi mostra el print de hopping(aixo nomes usar al server.c)
    ClientConf *conf = trobarSeguentMaester(r, t);
    if (!conf) {
        printString(1, "No s'ha trobat cap ruta per al destí.\n");
        return;
    }
    if(esHopping) {
        printMissHopping(conf, t->nom_origen, t->nom_desti);
    }
    pthread_t th;
    pthread_create(&th, NULL, runClient, conf);
    pthread_join(th, NULL); 
}


void *runClient(void *arg)
{
    ClientConf *conf = (ClientConf *)arg;
    int client_sock = create_client_socket(conf->ipDest, conf->portDest);
    if (client_sock < 0) {
        //printString(1, "Error creant el socket\n");
        free(conf);
        return NULL;
    }
    write(client_sock, conf->trama, sizeof(Trama));
    close(client_sock);
    free(conf);
    return NULL;
}

