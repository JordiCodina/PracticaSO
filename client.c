
#include "client.h"

#define BUFFER_SIZE 256

int create_client_socket(const char *ip, int port) {
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

void *waitAck( void *arg){//no s'utlitza aquesta funcio pero la deixem per si de cas
   // printf("adreçatlThreadArg: %p<-\n", arg);
   ThreadLog * tl = (ThreadLog * ) arg;
 //    printf("adreçatlThread: %p<-\n", &tl);

    Trama *response = malloc(sizeof(Trama)); 
   // printf("Waiting for ack on socket %d\n", tl->socket);
    read(tl->socket, response, sizeof(Trama)); // Dummy read to avoid compiler warning
    tl->thread_complete = 1;
    //return (void *) *response;
    return response;

}

Trama * waitForRead(double timeout,int socket,int* exitflag) {//no s'utlitza aquest funcio pero la deixem per si de cas

        time_t inici = time(NULL);    
        int cancelThread=0;
        ThreadLog tl; 
        tl.socket = socket;
        tl.thread_complete = 0;
        pthread_t waitAckThread;

        pthread_create(&waitAckThread, NULL,waitAck, &tl); 
       
        while(1){
            if(*exitflag==1){
                cancelThread=1;
                 pid_t pid = fork();
                if (pid == 0) {
                     pthread_kill(waitAckThread, SIGKILL);
                    _exit(0);   
                }
               
                return NULL;
            }
            sleep(1);
            time_t ara = time(NULL);
            double diff = difftime(ara, inici);
           // read(socket, response, sizeof(Trama));//esperem ack
            if (diff >= timeout ) {                      
                cancelThread =1;
                pthread_kill(waitAckThread, SIGKILL);
                break;
            }
            if (tl.thread_complete){
                break;
            }
        }
        Trama * t= NULL;
        if (cancelThread == 0)
        {
            pthread_join(waitAckThread, (void**)&t);            
            
        }else{
            t = NULL;
        }
        
        return t;

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
Trama * crearTrama(char tipus, char *ip, int port, char *nom_desti, unsigned short dataLength, char *data) {

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
        snprintf(t->data, sizeof(t->data), "%s", data);
    } else {
        t->data[0] = '\0';
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
    //int defaultIdx = -1;
    for (int i = 0; i < r->n_rutes; i++) {
        /*if(strcmp(r->rutes[i].nomDest, "DEFAULT") == 0){
            defaultIdx = i;
            printf("Default ")
        }*/
        if (strcmp(r->rutes[i].nomDest, t->nom_desti) == 0) {
            trobat = i;
            break;
        }
    }
    int idx = 0; 
    if (trobat >= 0 && !teCaracter(r->rutes[trobat].ipDest, '*')) {
        idx = trobat;
    }
   /* if (defaultIdx >= 0) {
        idx = defaultIdx;
    }*/
    /*if(trobat<0 && defaultIdx<0){
        ClientConf *conf = NULL;
        return conf;
    }*/
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


int  enviarTramaDirecte(Trama *t,int portDest,char *ipDest, char * nomDest){ 
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
    int socket=conectarClient_EnviarTrama_FerClose(conf);
    freeClientConf(conf);
    return socket;
}

 void enviarTramaDinamica(Trama *t, Regne *r, int esHopping) {//esHopping = 1 aixi mostra el print de hopping(aixo nomes usar al server.c)
    ClientConf *conf = trobarSeguentMaester(r, t);
    if (!conf) {
        printString(1, "No s'ha trobat cap ruta per al destí.\n");
        free(t);
        return;
    }
   
    
    if(esHopping) {
        printMissHopping(conf, t->nom_origen, t->nom_desti);
    }
    
    conectarClient_EnviarTrama_FerClose(conf);
     
      freeClientConf(conf);
}

int  enviarTramaDirecteSenseTancar(Trama *t,int portDest,char *ipDest, char * nomDest){ 
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

void enviarTramaSocket(Trama *t, int socket) {

    if(socket < 0 || t == NULL) {
        printString(1, "Socket no vàlid per enviar la trama.\n");
        return;
    }
    write(socket, t, sizeof(Trama));
}

void llegirTramaSocket(int socket, Trama *t) {
    read(socket, t, sizeof(Trama));
}


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
void writeTrama(int socket, Trama *t){
    write(socket, t, sizeof(Trama));
}

void rebreTrama(int socket){
    Trama *buffer = malloc(sizeof(Trama));
    if (buffer == NULL) {
        printString(1, "Error allocating memory for Trama buffer\n");
        return;
    }
    read(socket, buffer, sizeof(Trama));
    printTrama(*buffer);
    free(buffer);
    close(socket);
}*/



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
    
   /* Trama *response = malloc(sizeof(Trama)); 
    if (response == NULL) {
        printString(1, "Error allocating memory for Trama response\n");
        close(client_sock);
        return;
    }
    read(client_sock, response, sizeof(Trama));
    printf("Response received:\n");
    printTrama(*response);
    free(response);*/

    close(client_sock);
    return client_sock;

}
int conectarClient_EnviarTrama(ClientConf *conf)
{
    int client_sock = createClientSocket(conf);
    if (conf == NULL) {
        return -1;  
    }
    write(client_sock, conf->trama, sizeof(Trama));
   // printf("Llegint trama de resposta al client.c\n");
    /*Trama *response = malloc(sizeof(Trama)); 
    if (response == NULL) {
        printString(1, "Error allocating memory for Trama response\n");
        close(client_sock);
        return;
    }
    read(client_sock,response, sizeof(Trama));
   printTrama(*response);
   printf("Socket after reading trama, client.c: %d\n", client_sock);
    free(response);*/
  
    return client_sock;

}



