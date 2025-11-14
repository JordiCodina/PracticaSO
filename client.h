
#ifndef CLIENT_H
#define CLIENT_H

#include "funcions_basiques.h"
#include "config.h"
#define FRAME_SIZE 320
#define DATA_SIZE 275
typedef struct {
    uint8_t tipus;               
    char nom_origen[20];  // ip+port          
    char nom_desti[20];         
    uint16_t dataLength;          
    char data[DATA_SIZE];          
    uint16_t checksum;             
} Trama;

typedef struct {//ho poso tot junt ja que per el thread es mes facil
    int portDest;      // el port de destinació
    char *ipDest;      // l'adreça IP de destinació
    char *nomDest;     // el nom de destinació
    Trama trama;        //Trama a enviar
} ClientConf;

Trama * crearTrama(Regne * r,char tipus,char *nom_desti, char *data);
ClientConf *trobarSeguentSalt(Regne *r, Trama *t);
int create_client_socket(const char *ip, int port);

void* runClient(void *clientConf);
#endif