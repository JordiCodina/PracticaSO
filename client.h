
#ifndef CLIENT_H
#define CLIENT_H
#define _GNU_SOURCE 
#include <pthread.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "funcions_basiques.h"
#include "config.h"
#define FRAME_SIZE 320
#define DATA_SIZE 275
typedef struct {
    unsigned char tipus;               
    char nom_origen[20];  // ip+port          
    char nom_desti[20];         
    unsigned short dataLength;          
    char data[DATA_SIZE];          
    unsigned short checksum;             
} Trama;

typedef struct {//ho poso tot junt ja que per el thread es mes facil
    int portDest;      // el port de destinació del seguent hop
    char *ipDest;      // l'adreça IP de destinació del seguent hop
    char *nomDest;     // el nom de destinació del seguent hop
    Trama *trama;        //Trama a enviar
    int socket;
} ClientConf;

Trama * crearTrama(char tipus,char *ip,int port,char *nom_desti,unsigned short dataLength,char *data);
ClientConf *trobarSeguentMaester(Regne *r, Trama *t);
void enviarTrama(Trama *t, Regne *r, int esHopping) ;//aixo es mes per si hi ha un possible hopping 
void enviarTramaDirecte(Trama *t,int portDest,char *ipDest, char * nomDest);//enviament directe sense hopping si saps ja tot

int create_client_socket(const char *ip, int port);
void conectarClient_EnviarTrama_FerClose(ClientConf *conf);

void printMissHopping(ClientConf *conf, char *nomOrg, char *nomDest);
#endif