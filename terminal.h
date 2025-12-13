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



void runTerminal(Regne *r, char *fitxerStock);




#endif