
#include "client.h"

#define BUFFER_SIZE 256

int create_client_socket(const char *ip, unsigned int port) {
    int client_sock;
    struct sockaddr_in serv_addr;
    
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0) {
        printString(1, SOCKET_CREATION_FAILED);
        return -1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        printString(1, INVALID_ADDRESS);
        close(client_sock);
        return -1;
    }
    
    if (connect(client_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printString(1, CONNECTION_FAILED);
        close(client_sock);
        return -1;
    }
    
    return client_sock;
}

void freeClientConf(ClientConf *conf) {
    if (conf == NULL) return;
    if (conf->ipDest != NULL) {
        free(conf->ipDest);
        conf->ipDest = NULL;
    }
    if (conf->nomDest != NULL) {
        free(conf->nomDest);
        conf->nomDest = NULL;
    }
    if (conf->trama != NULL) {
        free(conf->trama);
        conf->trama = NULL;
    }
    if (conf->socket > 0) {
        conf->socket = -1;
    }
    free(conf);
}

void printMissHopping(ClientConf *conf, char *nomOrg, char *nomDest) {
    char *miss;
    asprintf(&miss,">>> Recived hop with ip:port -> %s to realm name -> %s\n Found route: %s -> %s:%d \n Forwarding PLEDGE request\n", nomOrg, nomDest, conf->nomDest, conf->ipDest,conf->portDest);
    printString(1, miss);
    free(miss);

}

unsigned short calcularChecksum(Trama *t) {
    unsigned long suma = 0;  
    unsigned char *ptr = (unsigned char *)t;
    for (size_t i = 0; i < sizeof(Trama); i++) {
        if (ptr + i >= (unsigned char *)&t->checksum && ptr + i <  (unsigned char *)&t->checksum + sizeof(t->checksum)) {
            continue;
        }
        suma += ptr[i];
    }
    return (unsigned short)(suma % 65536);
}
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
Trama * crearTrama(char tipus, char *ip, unsigned int port, char *nom_desti, unsigned short dataLength, char *data) {

    Trama * t = malloc (sizeof(Trama));
    if (t == NULL) {
        return NULL;
    }
    memset(t, 0, sizeof(Trama));
    t->tipus = tipus;
    if (ip != NULL) {
        snprintf(t->nom_origen, sizeof(t->nom_origen), "%s:%d", ip, port);
    } else {
        t->nom_origen[0] = '\0';
    }
    if (nom_desti != NULL) {
        snprintf(t->nom_desti, sizeof(t->nom_desti), "%s", nom_desti);
    } else {
        t->nom_desti[0] = '\0';
    }
    if (data != NULL) {
        size_t copySize = dataLength;
        if (copySize > sizeof(t->data)) copySize = sizeof(t->data); 
        memcpy(t->data, data, copySize);
    }
    if (tipus == 0) {
        t->checksum = 0;
        t->dataLength = 0;
        return t;
    }
    t->dataLength = dataLength;
    t->checksum = calcularChecksum(t); 
    
    return t;
}

//aquesta funcio busca la seguent ruta (tant si esta directament conectada com si no i llavors s'ha de fer salt) 
ClientConf *trobarSeguentMaester(Regne *r, Trama *t) {//si troba ip amb * , agafa default
    int trobat = -1;
    for (int i = 0; i < r->n_rutes; i++) {
        if (strcmp(r->rutes[i].nomDest, t->nom_desti) == 0) {
            trobat = i;
            break;
        }
    }
    int idx = 0; 
    if (trobat >= 0 && !teCaracter(r->rutes[trobat].ipDest, '*')) {
        idx = trobat;
    }
    ClientConf *conf = malloc(sizeof(ClientConf));
    memset(conf, 0, sizeof(ClientConf));
    if (!conf) {
        printString(1, "Error allocating memory for ClientConf\n");
        return NULL;
    }
     memset(conf, 0, sizeof(ClientConf));
    conf->ipDest   = strdup(r->rutes[idx].ipDest); 
    conf->nomDest  = strdup(r->rutes[idx].nomDest);
    conf->portDest = r->rutes[idx].portDest; 
    conf->trama = t;                    
    return conf;
}

/*
@brief Envia una trama directament a un destí específic tancant el socket.
@param t Punter a la trama a enviar.
@param portDest El port del destí.
@param ipDest La direcció IP del destí.
@param nomDest El nom del destí.
@return Enter que indica l'estat de l'enviament (0 èxit, -1 error).
*/

int  enviarTramaDirecte(Trama *t,unsigned int portDest,char *ipDest, char * nomDest){ 
    ClientConf *conf = malloc(sizeof(ClientConf));
    if (!conf) {
        printString(1, "Error allocating memory for ClientConf\n");
        free(t);
        return -1;
    }
    memset(conf, 0, sizeof(ClientConf));
    conf->portDest = portDest;
    conf->ipDest   = strdup(ipDest);   
    conf->nomDest  = strdup(nomDest);  
    conf->trama    = t;                    
    int status=conectarClient_EnviarTrama_FerClose(conf);
    freeClientConf(conf);
    return status;
}

/*
@brief Envia una trama directament a un destí específic sense tancar el socket.
@param t Punter a la trama a enviar.
@param portDest El port del destí.
@param ipDest La direcció IP del destí.
@param nomDest El nom del destí.
@return Enter que indica el socket utilitzat per l'enviament.
*/

int  enviarTramaDirecteSenseTancar(Trama *t, unsigned int portDest,char *ipDest, char * nomDest){ 
    ClientConf *conf = malloc(sizeof(ClientConf));
    if (!conf) {
        printString(1, "Error allocating memory for ClientConf\n");
        free(t);
        return -1;
    }
     memset(conf, 0, sizeof(ClientConf));

    conf->portDest = portDest;
    conf->ipDest   = strdup(ipDest);   
    conf->nomDest  = strdup(nomDest);  
    conf->trama    = t;                    
    int socket=conectarClient_EnviarTrama(conf);

    freeClientConf(conf);
    return socket;
}

/*
@brief Envia una trama dinamica per hops ,automaticament va a la taula de rutes per trobar la seguent ruta , si no troba doncs default
@param t Punter a la trama a enviar.
@param r Punter a la estructura Regne que conté la informació del regne actual.
@param esHopping Enter que indica si s'ha de mostrar missatge de hopping (1 sí, 0 no).
*/

int enviarTramaDinamica(Trama *t, Regne *r, int esHopping) {//esHopping = 1 aixi mostra el print de hopping(aixo nomes usar al server.c)
    ClientConf *conf = trobarSeguentMaester(r, t);
    if (!conf) {
        printString(1, "No s'ha trobat cap ruta per al destí.\n");
        free(t);
        return -1;
    }
   
    
    if(esHopping) {
        printMissHopping(conf, t->nom_origen, t->nom_desti);
    }
    int status = conectarClient_EnviarTrama_FerClose(conf);
     
    freeClientConf(conf);
    return status;
}

/*
@brief Envia una trama a través d'un socket específic.
@param t Punter a la trama a enviar.
@param socket El socket a través del qual s'envia la trama.
*/

void enviarTramaSocket(Trama *t, int socket) {

    if(socket < 0 || t == NULL) {
        printString(1, "Socket no vàlid per enviar la trama.\n");
        return;
    }
    write(socket, t, sizeof(Trama));
}

/*
@brief Crea un socket client basat en la configuració proporcionada.
@param conf Punter a la configuració del client que conté la informació de la connexió
@return Enter que indica el descriptor del socket creat (o -1 en cas d'error).
*/

int createClientSocket(ClientConf *conf)
{

    if (conf == NULL) {
        printString(1, "ClientConf és NULL\n");
        return -1;
    }
    int client_sock = create_client_socket(conf->ipDest, conf->portDest);
    if (client_sock < 0) {
       // printString(1, "Error creant el socket client\n");
        return -1;
    }
    return client_sock;
}

/*
@brief Conecta al client, envia la trama i tanca el socket.
@param conf Punter a la configuració del client que conté la informació de la connexió
@return Enter que indica l'estat de l'enviament (0 èxit, -1 error).
*/

int conectarClient_EnviarTrama_FerClose(ClientConf *conf)
{
  

    if (conf == NULL) {
        return -1;
    }
    int client_sock = createClientSocket(conf);
    if (client_sock < 0) {
        return -1;  
    }
    write(client_sock, conf->trama, sizeof(Trama));
    close(client_sock);
    return client_sock;

}
int conectarClient_EnviarTrama(ClientConf *conf)
{

    int client_sock = createClientSocket(conf);
    if (conf == NULL) {
        printString(1, "ClientConf és NULL\n");
        return -1;  
    }
    write(client_sock, conf->trama, sizeof(Trama));
  
    return client_sock;

}



