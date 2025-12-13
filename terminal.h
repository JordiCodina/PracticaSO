#ifndef TERMINAL_H
#define TERMINAL_H

#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include "funcions_basiques.h"

#include "regne.h"
#include "producte.h"
#include "client.h"
#include "server.h"
#include "config.h"
#define COM_LEN 256

typedef struct _ThreadReadTerminal {//no sutiltiza aquesta funcio pero la deixem per si de cas
    char thread_complete;
    char *comanda;
} ThreadReadTerminal;

char * waitForExit(int *exitflag);//no s'utlitza aquesta funcio pero la deixem per si de cas

void runTerminal(Regne *r, char *fitxerStock);




#endif