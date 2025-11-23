

#include "server.h"

int create_server_socket(int port) {
    int server_fd;
    struct sockaddr_in address;
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        printString(1,ERROR_SOCKET);
        return -1;
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        printString(1,ERROR_BIND);
        close(server_fd);
        return -1;
    }
    
    if (listen(server_fd, 3) < 0) {
        printString(1,ERROR_LISTEN);
        close(server_fd);
        return -1;
    }
    
    return server_fd;
}
int accept_client_connection(int server_fd) {
    int new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t*) &addrlen);
    if (new_socket < 0) {
        //printString(1,ERROR_ACCEPT);
        return -1;
    }
    
    return new_socket;
}

/*

RESPONDRE A PETCIONS DEL CLIENT COM A SERVIDOR

*/

 void processarTramaPledge(Trama *t, Regne *r,int socket) {
    int n_paraules = 0;
    int n_paraules2 = 0;
    int trobat=0;
    char **paraules  = tokenizer(t->data, &n_paraules, '&');
    char **paraules2 = tokenizer(t->nom_origen, &n_paraules2, ':');//ip:port
    if (n_paraules != 4) {
        freeTokens(paraules, n_paraules);
        return;
    }
    for (int i = 0; i < r->n_rutes; i++) {
        if (strcmp(r->rutes[i].nomDest, paraules[0]) == 0) {
            r->rutes[i].alianca = 'P';
            if (r->rutes[i].ipDest) {
                free(r->rutes[i].ipDest);
            }
            r->rutes[i].ipDest =strdup(paraules2[0]);
            r->rutes[i].portDest = atoi(paraules2[1]);
            trobat=1;
            printString(1, ">>> Alliance request received from ");
            printString(1, paraules[0]);
            printString(1, ".\n");
            break;
        }
    }    
    if(trobat){
        Trama * ackTrama = crearTrama(0x31, NULL, 0, NULL, 2, "OK");
        write(socket, ackTrama, sizeof(Trama));
        free(ackTrama);
    }
    freeTokens(paraules, n_paraules);
    freeTokens(paraules2, n_paraules2);
}

 void processarTramaPledgeRespond(Trama *t, Regne *r,int client_sock) {

    int n_paraules = 0;
    int currRuta=0;
    char **paraules = tokenizer(t->nom_origen, &n_paraules, ':');
    if (n_paraules != 2) {
        freeTokens(paraules, n_paraules);
        printString(1, "Pledge respond trama malformed\n");
        return;
    }
    if (strcasecmp(t->data, "ACCEPT") == 0) {
        for (int i = 0; i < r->n_rutes; i++) {
            if (strcmp(r->rutes[i].nomDest, t->nom_desti) == 0) {
                currRuta=i;
                if (r->rutes[i].alianca == 'T') {
                    break;
                }
                r->rutes[i].alianca = 'S';
                //if (r->rutes[i].esDirectamentConectat == 0) {
                    r->rutes[i].portDest = atoi(paraules[1]);
                    if (r->rutes[i].ipDest) {
                        free(r->rutes[i].ipDest);
                    }
                    r->rutes[i].ipDest = strdup(paraules[0]);                  
                //}
                char *buffer;
                asprintf(&buffer, ">>> Alliance with %s forged successfully!\n", r->rutes[i].nomDest);
                printString(1, buffer);
                free(buffer);
                break;
            
            }
        }
    } else if (strcasecmp(t->data, "REJECT") == 0) {
        for (int i = 0; i < r->n_rutes; i++) {
            if (strcmp(r->rutes[i].nomDest, t->nom_desti) == 0) {
                 currRuta=i;
                if (r->rutes[i].alianca == 'T') {
                    break;
                }
                r->rutes[i].alianca = 'N';
                printString(1, "Alliance has been rejected.\n");
                break;
            }
        }
    }
    if(r->rutes[currRuta].alianca=='T'){
        Trama * nackTrama=crearTrama(0x69, NULL, 0, NULL, 0, NULL);
        //printf("Sending NACK trama to client socket (servidor.c) %d\n", client_sock);
        write(client_sock, nackTrama, sizeof(Trama));
        free(nackTrama);

    }else if(r->rutes[currRuta].alianca=='S'){
        Trama * ackTrama = crearTrama(0x31, NULL, 0, NULL, 2, "OK");
       // printf("Sending ACK trama to client socket (servidor.c) %d\n", client_sock);
        write(client_sock, ackTrama, sizeof(Trama));
        free(ackTrama);
    }
   // enviarTramaACKFitxer(paraules[0], atoi(paraules[1]), t->nom_desti, "OK",client_sock);
    freeTokens(paraules, n_paraules);
}


void processarTramaLlistarProductes(Trama *t, Regne *r,int client_sock) {

       printString(1, "LIST PRODUCTS request from ");
        printString(1, t->nom_origen);
        for(int i=0;i<r->n_rutes;i++){
            if(strcmp(r->rutes[i].nomDest,t->data)==0){
                if(r->rutes[i].alianca!='S'){
                        Trama * authtrama = crearTrama(0x25, r->ip, r->port, t->data, 5, "AUTH");
                        enviarTramaSocket(authtrama, client_sock);
                        free(authtrama);
                        printString(1, "You are not allied with this Realm or it is inactive. Sending AUTH NACK.\n");
                    return;
                }
                break;
            }
        }
        printString(1, ".\nSending product list...\n");

       char * filename=malloc(FILENAME_STOCK_SIZE*sizeof(char));
        snprintf(filename, FILENAME_STOCK_SIZE, "stock%s.db",r->nom);
        int fd=open(filename, O_RDONLY);
        if(fd==-1){//cas impossible ja que esta harcodeat
            printString(1, "Error opening  file\n");
            Trama * nackTrama=crearTrama(0x69, NULL, 0, NULL, 0, NULL);
            enviarTramaSocket(nackTrama, client_sock);
            free(nackTrama);
            free(filename);
            return;
        }
        int size=getSize(fd);
        char data[DATA_SIZE];
        //DATA: <FileName>&<FileSize>&<MD5SUM>
        snprintf(data, DATA_SIZE, "%s&%d&%s",filename,size,"MD5SUM");
        free(filename);
        Trama * trama = crearTrama(0x12, r->ip, r->port, r->nom , strlen(data),data);
        write(client_sock, trama, sizeof(Trama));
        printString(1, "Products delivered.\n");
        free(trama);
        close(fd);
        
}

/*

AQUI ES PROCESSEN TOTES LES TRAMES A PARTIR DEL TIPUS

*/
 void processarTrama(Trama *t, Regne *r,int client_sock) {

    switch(t->tipus){

        case 0x01: 
                int n_paraules = 0;
                char **paraules = tokenizer(t->nom_origen, &n_paraules, ':');
                //enviarTramaACKFitxer(paraules[0], atoi(paraules[1]), t->nom_desti, "OK", client_sock);
                freeTokens(paraules, n_paraules);
              
                processarTramaPledge(t, r, client_sock); 
                break;
        
            case 0x02:
                printString(1, "Trama segell  rebuda:\n");
             
                break;
            case 0x03:
                processarTramaPledgeRespond(t, r, client_sock);
                
                break;
            case 0x11: 
                processarTramaLlistarProductes(t, r, client_sock);
                
                break;
            case 0x12:
                printString(1, "Trama resposta llistar productes rebuda:\n");
               
                break;
            case 0x13:
                printString(1, "Trama envia dades producte rebuda:\n");
                
                break;
            case 0x14:
                printString(1, "Trama shoping list header rebuda:\n");
               
                break;
            case 0x15:

                printString(1, "Trama enviament de fitxer productes rebuda:\n");
              
                break;
            case 0x16:
                printString(1, "Trama  rebuda:\n");
              
                break;
            case 0x27:
                int n_paraules3 = 0;
                char **paraules3 = tokenizer(t->nom_origen, &n_paraules3, ':');
                int port= atoi(paraules3[1]);
                for (int i = 0; i < r->n_rutes; i++) {
                        if(r->rutes[i].portDest==port && strcmp(r->rutes[i].nomDest,"DEFAULT")!=0){
                        if(r->rutes[i].alianca == 'S'){
                            r->rutes[i].alianca = 'I';
                            printString(1, "Alliance has been disconnected.\n");
                        }
                       
                        break;
                    }
                }

                freeTokens(paraules3, n_paraules3);
                break;
            case 0x21:
                printString(1, "Trama regne desconegut rebuda:\n");
              
                break;
            case 0x25:
                printString(1, "Trama no aliança per productes rebuda:\n");
              
                break;
            case 0x26:
                printString(1, "Trama ping rebuda:\n");
                
                break;
            case 0x31:
                /*if(strcmp(t->data,"OK")){
                printString(1, "Trama ack(OK) rebuda:\n");
                }else{
                    printString(1, "Trama ack(KO) rebuda:\n");
                }*/
                
                break;
            case 0x32:
                printString(1, "Trama md5sum rebuda:\n");
             
                break;
            case 0x69:
                printString(1, "Trama error : NACK  rebuda:\n");
                
                break;           
            default:
                printString(1, "Tipus de trama desconegut.\n");
                break;
        }
    }


int tocaIgnorarTrama(int tipus){
    if(tipus==0x69 || tipus==0x31 || tipus==0x32 || tipus==0x03){//poden haver-hi mes
        return 1;
    }
    return 0;
}
int comprovarChecksum(Trama *t){
    unsigned short checksumCalculat = calcularChecksum(t);
    if (checksumCalculat != t->checksum) {
        return 0;
    }
    return 1;
}

int tocaFinalitzarServidor(int tipus){
    if(tipus==0x27){
        return 1;
    }
    return 0;
}

void* runServer(void *aregne) {
    Regne *r = (Regne *)aregne;
    r->fd_server = create_server_socket(r->port);
    if (r->fd_server < 0) {
        printString(1, "Error creant el servidor\n");
        return (void*)1;
    }

    // posar el socket en mode no bloquejant perque si no es quedaria aqui tota la estona
    //no puc fer un kill del thread amb fork perque quedaria memoria dinamica i sockets oberts
    int flags = fcntl(r->fd_server, F_GETFL, 0);
    fcntl(r->fd_server, F_SETFL, flags | O_NONBLOCK);
    while (*r->exit==0) {

        int new_socket = accept_client_connection(r->fd_server);
        if (new_socket < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(100000); 
                continue;
            } else {
                break;
            }
        }
        int cflags = fcntl(new_socket, F_GETFL, 0);
        fcntl(new_socket, F_SETFL, cflags | O_NONBLOCK);
        
        Trama* t = crearTrama(0, "", 0, "", 0, ""); //inicialitzar trama
        ssize_t servRead = read(new_socket, t, sizeof(Trama));
            if (servRead <= 0) {
                printString(1, "Connexió tancada o error.\n");
                close(new_socket);
                free(t);
                break;
            }
            if (servRead != sizeof(Trama)) {
                printString(1, "Trama rebuda incompleta.\n");
                close(new_socket);
                free(t);
                continue;
            }
            if(tocaIgnorarTrama(t->tipus)){//ignorar trames de tipus ACK,NACK,MD5SUM i tal , que les processi directament sense passar perl passos seguents,sense fer checksum ni que les reenvii
                processarTrama(t, r, new_socket);
                close (new_socket);
                free(t);
                continue;
            }
            if (!comprovarChecksum(t) && t->checksum>0) {//compovem checksum
                    int n_paraules = 0;
                    char **paraules = tokenizer(t->nom_origen, &n_paraules, ':');
                    Trama * tramaNACK=crearTrama(0x69, NULL, 0, NULL,0, NULL);//crear trama NACK
                    enviarTramaDirecte(tramaNACK, atoi(paraules[1]), paraules[0], t->nom_desti);//trama s'allibera aqui
                    freeTokens(paraules, n_paraules);
                    close(new_socket);
                    free(t);
                    continue;
            }

            if (strcmp(t->nom_desti, r->nom) == 0) {
                //es el destinatari final
                processarTrama(t, r, new_socket);
                free(t);
            }else {
                //aqui fa hopping
                enviarTramaDinamica(t, r, 1);//s'allibera t a dins
            }
            close(new_socket);

            
    }
    close(r->fd_server);
    //printString(1, "Maester: Socket tancat\n");
    return NULL;
}


void closeServer(Regne *r){//per cntol +c
    if (r->fd_server > 0) {
     close(r->fd_server);
    }
}
