
#ifndef CONFIG_H
#define CONFIG_H


#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "funcions_basiques.h"


typedef struct
{
    char name[100];
    int amount;
    float weight;
} Producte;


typedef struct {

    int status; // 0 = lliure, 1 = ON MISSION
    char *nomRegneViatge;
    int pipeRead;
    int pipeWrite;
    pid_t pid;
} Envoy;

typedef struct {

    char *nomDest;
    char *ipDest;
    unsigned int portDest;
    int esDirectamentConectat; // 1 -> sí, 0 -> no // es conegut no es el mateix que alianca , es per el cas de les ip amb *.*.*.* 0 de la configuracio inical.Llavors si es 0 representa  que si fem una aliança = 'S' haurem de substiruir aquest valors de ip i port, si no no cal
    char alianca; // "S" o "N" // si/no son aliats ,o "P" // 'O' // s'ha fet una request // 'T' timeout // 'I' inactiu 

    int sigilSize;

    char MD5SUM[33];

    int ackFitxerStatus;
    int ackMD5SUMStatus;

} Ruta;

typedef struct serverThread {
    pthread_t thread_id;
    int thread_complete;
    int client_fd;
} ServerThread;

typedef struct  {

    char *nom;
    char *rutaArxius;
    char * ip;
    unsigned int port;

    Envoy *envoys;
    int n_envoys;

    Ruta *rutes;
    int n_rutes;

    int fd_server;

    int *exit;

    ServerThread **st;
    int n_serverThreads;

} Regne;

//funcions d'alliberar memòria dinàmica

void freeEnvoy(Envoy e);

void freeRuta(Ruta r);

void freeRegne(Regne *r);



//funcions d'inicialitzar els strucs

void initializeEnvoy (Envoy *e);

void initializeRuta (Ruta *r);

void initializeRegne (Regne *r);



//funcions per copiar els structs 

void envoyCpy (Envoy *dest, Envoy *org);

void rutaCpy (Ruta *dest, Ruta *org);

void regneCpy (Regne *dest, Regne* org);

void cpyProducte (Producte *dest, Producte *org);



//funcions de envoy

/*
@brief Posa un estatus a un envoy
@param *e el envoy
@param *text el estatus que volem posar
*/

void setStatusEnvoy(Envoy *e, const char *text);

/*
@brief reseteja un evoy
@param *e el envoy a resetejar
*/

void resetEnvoy(Envoy *e) ;



/*
@brief Mira si un Maester existeix
@param *r el teu regne
@param *nomRegneRuta el Maester que volem saber si existeix
@return 0 = No existeix, 1 = existeix
*/

int comprovarRegneExisteix(Regne *r, char * nomRegneRuta);

//funcions per extreure la informació dels fitxers

/*
@brief Posa l'informació que troba en el fitxer .dat 
@param nomFitxer el nom del fitxer de configuració
@return Regne el regne amb tot.
*/

Regne *extractInfo (const char *nomFitxer);

/*
@brief Extreu tots els productes
@param fProductes el file descriptor del fitxer
@param *n_productes punter a la quantitat de productes
@return Producte* punter als productes
*/

Producte *extreureProductes(int fProductes, int *n_productes);

#endif