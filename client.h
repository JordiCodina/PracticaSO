
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
    unsigned int portDest;      // el port de destinació del seguent hop
    char *ipDest;      // l'adreça IP de destinació del seguent hop
    char *nomDest;     // el nom de destinació del seguent hop
    Trama *trama;        //Trama a enviar
    int socket;
} ClientConf;

typedef struct _ThreadLog {//no sutiltiza pero ho deixem temporalment per si de cas

    char thread_complete;
    int socket;

} ThreadLog;


unsigned short calcularChecksum(Trama *t) ;

/*
@brief Crea una nova trama amb els camps especificats.
@param tipus El tipus de la trama.
@param ip La direcció IP de l'origen.)
@param port El port de l'origen.
@param nom_desti El nom del destí.
@param dataLength La longitud de les dades.
@param data Les dades de la trama.
@return Punter a la nova trama creada.
*/

Trama * crearTrama(char tipus,char *ip,unsigned int port,char *nom_desti,unsigned short dataLength,char *data);

//aquesta funcio busca la seguent ruta (tant si esta directament conectada com si no i llavors s'ha de fer salt) 

ClientConf *trobarSeguentMaester(Regne *r, Trama *t);

/*
@brief Envia una trama directament a un destí específic tancant el socket.
@param t Punter a la trama a enviar.
@param portDest El port del destí.
@param ipDest La direcció IP del destí.
@param nomDest El nom del destí.
@return Enter que indica l'estat de l'enviament (0 èxit, -1 error).
*/

int enviarTramaDirecte(Trama *t,unsigned int portDest,char *ipDest, char * nomDest);//enviament directe sense hopping si saps ja tot

/*
@brief Envia una trama directament a un destí específic sense tancar el socket.
@param t Punter a la trama a enviar.
@param portDest El port del destí.
@param ipDest La direcció IP del destí.
@param nomDest El nom del destí.
@return Enter que indica el socket utilitzat per l'enviament.
*/

int  enviarTramaDirecteSenseTancar(Trama *t,unsigned int portDest,char *ipDest, char * nomDest);

/*
@brief Envia una trama dinamica per hops ,automaticament va a la taula de rutes per trobar la seguent ruta , si no troba doncs default
@param t Punter a la trama a enviar.
@param r Punter a la estructura Regne que conté la informació del regne actual.
@param esHopping Enter que indica si s'ha de mostrar missatge de hopping (1 sí, 0 no).
*/

int enviarTramaDinamica(Trama *t, Regne *r, int esHopping) ;//aixo es mes per si hi ha un possible hopping 

/*
@brief Envia una trama a través d'un socket específic.
@param t Punter a la trama a enviar.
@param socket El socket a través del qual s'envia la trama.
*/

void  enviarTramaSocket(Trama *t, int socket) ;

/*
@brief Conecta al client, envia la trama i tanca el socket.
@param conf Punter a la configuració del client que conté la informació de la connexió
@return Enter que indica l'estat de l'enviament (0 èxit, -1 error).
*/

int conectarClient_EnviarTrama_FerClose(ClientConf *conf);

int conectarClient_EnviarTrama(ClientConf *conf);


int ferReadSocketSerialitzat(int socket, Trama *t);

#endif