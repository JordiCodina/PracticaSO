
#ifndef CLIENT_H
#define CLIENT_H
#define _GNU_SOURCE 

#include <netdb.h>
#include <arpa/inet.h> 
#include <sys/socket.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "funcions_basiques.h"
#include "config.h"

#define WON_MESSAGE "¡Has ganado!\n"
#define LOST_MESSAGE "¡Juego terminado! Has perdido.\n"
#define INVALID_ADDRESS "Dirección inválida\n"
#define CONNECTION_FAILED "Can't connect to realm\n"
#define SOCKET_CREATION_FAILED "Falló la creación del socket\n"
#define ASK_LETTER "Introduce una letra: "
#define INVALID_INPUT "Entrada inválida. Solo se puede introducir una letra\n"
#define USAGE "Uso: ./client <ip> <puerto>\n"




#define FRAME_SIZE 320
#define DATA_SIZE 275
typedef struct {
    unsigned char tipus;               
    char nom_origen[20];  // ip:port          
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

typedef struct _ThreadLog {//no sutiltiza pero ho deixem temporalment per si de cas

    char thread_complete;
    int socket;

} ThreadLog;



//Trama * waitForRead(double timeout,int socket,int* exitflag);

  void freeClientConf(ClientConf *conf);

Trama * crearTrama(char tipus,char *ip,int port,char *nom_desti,unsigned short dataLength,char *data);
unsigned short calcularChecksum(Trama *t);


ClientConf *trobarSeguentMaester(Regne *r, Trama *t);

int enviarTramaDinamica(Trama *t, Regne *r, int esHopping) ;//aixo es mes per si hi ha un possible hopping 
int enviarTramaDirecte(Trama *t,int portDest,char *ipDest, char * nomDest);//enviament directe sense hopping si saps ja tot
int  enviarTramaDirecteSenseTancar(Trama *t,int portDest,char *ipDest, char * nomDest);
void  enviarTramaSocket(Trama *t, int socket) ;

int create_client_socket(const char *ip, int port);

int conectarClient_EnviarTrama_FerClose(ClientConf *conf);
int conectarClient_EnviarTrama(ClientConf *conf);


void printMissHopping(ClientConf *conf, char *nomOrg, char *nomDest);
#endif