

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
        printString(1,ERROR_ACCEPT);
        return -1;
    }
    
    return new_socket;
}

void printTrama(Trama t){//aixo es per debugar pero es borrara
        printf("─────────────\n");
        printf("Tipus: 0x%02X\n", t.tipus);
        printf("Origen(ip+port): %s\n", t.nom_origen);
        printf("NomDestí: %s\n", t.nom_desti);
        printf("Longitud dades: %d\n", t.dataLength);
        printf("Dades: %s\n", t.data);
        printf("Checksum: %u (OK)\n", t.checksum);
        printf("─────────────\n");
} 


/*

ENVAIMENT TRAMES ESPECÍFIQUES

*/

void enviarTramaACKFitxer(char * ip, int port, char *nomDest,char * status ,int client_sock ){//status es OK o KO
    printf("Enviant ACK de fitxer a %s:%d per %s amb estat %s\n", ip, port, nomDest, status);
    Trama * tramaNACK=crearTrama(0x31, NULL, 0, NULL,2, status);
    //sleep(4);//sleep 4 segons
    
    //enviarTramaDirecte(tramaNACK, port, ip, nomDest);
   // free(tramaNACK);
    write(client_sock, tramaNACK, sizeof(Trama));
    free(tramaNACK);

}
void enviarTramaNACK(char * ip, int port, char *nomDest ){
    Trama * tramaNACK=crearTrama(0x69, NULL, 0, NULL,0, NULL);
    enviarTramaDirecte(tramaNACK, port, ip, nomDest);
    free(tramaNACK);
}
/*void enviarTramaMD5SUM(Regne *r, unsigned short llargariaDades){

    Trama * tramaMD5SUM=crearTrama(0x32, NULL, 0, NULL, llargariaDades, "CHECK_OK");
    enviarTramaDirecte(tramaMD5SUM, r->port, r->ip, "MDSUM5");
    free(tramaMD5SUM);
}*/


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
    }
    freeTokens(paraules, n_paraules);
    freeTokens(paraules2, n_paraules2);
}

 void processarTramaPledgeRespond(Trama *t, Regne *r,int client_sock) {
    printTrama(*t);
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
                if (r->rutes[i].esDirectamentConectat == 0) {
                    r->rutes[i].portDest = atoi(paraules[1]);
                    r->rutes[i].ipDest   = strdup(paraules[0]);                   
                }
                printString(1, "Alliance has been accepted.\n");
                
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
       printString(1, "Sending product list.Products delivered.");
        printString(1, "\n");        
       char * filename=malloc(FILENAME_STOCK_SIZE*sizeof(char));
        //snprintf(filename, FILENAME_STOCK_SIZE, "%s.db",r->nom);
        snprintf(filename, FILENAME_STOCK_SIZE, "stock%s.db",r->nom);
        int fd=open(filename, O_RDONLY);
        if(fd==-1){
            printString(1, "Error opening  file\n");
            free(filename);
            close(fd);
            return;
        }
      
        int size=getSize(fd);
        char data[DATA_SIZE];
        //DATA: <FileName>&<FileSize>&<MD5SUM>
        snprintf(data, DATA_SIZE, "%s&%d&%s",filename,size,"MD5SUM");
        free(filename);
        Trama * trama = crearTrama(0x12, r->ip, r->port, r->nom , strlen(data),data);
        write(client_sock, trama, sizeof(Trama));
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
                printString(1, "Trama llistar productes rebuda:\n");
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
                printString(1, "Trama desconexió rebuda:\n");
                //marcarAliançaComDesconectada(r, t->nom_origen);
                int n_paraules3 = 0;
                char **paraules3 = tokenizer(t->nom_origen, &n_paraules3, ':');
                int port= atoi(paraules3[1]);
                for (int i = 0; i < r->n_rutes; i++) {
                        if(r->rutes[i].portDest==port) {
                        if(r->rutes[i].alianca = 'S'){
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
    while (1) {
        //printString(1, "Maester: Esperant connexions...\n");
        int new_socket = accept_client_connection(r->fd_server);
        if (new_socket < 0) {
            printString(1, "Error acceptant connexió\n");
            continue;
        }
        
        Trama* t = crearTrama(0, "", 0, "", 0, ""); //inicialitzar trama
        ssize_t servRead = read(new_socket, t, sizeof(Trama));
            if (servRead <= 0) {
                printString(1, "Connexió tancada o error.\n");
                close(new_socket);
                break;
            }
            if (servRead != sizeof(Trama)) {
                printString(1, "Trama rebuda incompleta.\n");
                close(new_socket);
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
                    //enviarTramaACKFitxer(paraules[0], atoi(paraules[1]), t->nom_desti, "KO");
                    enviarTramaNACK(paraules[0], atoi(paraules[1]), t->nom_desti);
                    freeTokens(paraules, n_paraules);
                    close(new_socket);
                    free(t);
                    continue;
            }

            //aqui haurem de comprovar el MD5SUM 0x32
            
            /*if(tocaFinalitzarServidor(t->tipus)){
                //si es una trama que tanca connexio
                \n");
                close(new_socket);
                free(t);
                break;

            }*/

            if (strcmp(t->nom_desti, r->nom) == 0) {//aqui es el desti final
                processarTrama(t, r, new_socket);
            }else {//aqui fa hopping
                enviarTramaDinamica(t, r, 1);
            }
            close(new_socket);

            free(t);
    }
    close(r->fd_server);
    printString(1, "Maester: Socket tancat\n");
    return NULL;
}


void closeServer(Regne *r){
    close(r->fd_server);
}
