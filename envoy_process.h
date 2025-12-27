#ifndef _ENVOYPROCESS_H_
#define _ENVOYPROCESS_H_

#define _GNU_SOURCE

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

#include "regne.h"

void runenvoy( int pipeRead, int pipeWrite,Regne *r) ;
#endif