
#ifndef CLIENT_H
#define CLIENT_H

#include "funcions_basiques.h"
int create_client_socket(const char *ip, int port);

int runClient(int port, char *ip);
#endif