#ifndef _REGNE_H_
#define _REGNE_H_

#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "funcions_basiques.h"
#include "config.h"
#include "client.h"
#include "server.h"



int comprovarRutaRegneValid(char *str,int n_rutes , Ruta *rutes);

// Comanda LIST REALMS

void listRealms(Regne *r);

// Comanda PLEDGE
void sendEnvoy(Regne *r, char *casa);
//void pledge (Regne *r, char comanda [100]);
void pledge (Regne *r,char **paraules);
// Comanda PLEDGE RESPOND
void pledgeRespond(Regne *r, char **paraules, int n_paraules);
// Comanda PLEDGE STATUS 
void pledgeStatus (Regne *r);
// Comanda ENVOY STATUS
void envoyStatus (Regne *r);



#endif