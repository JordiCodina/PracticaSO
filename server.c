

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

void rebreSigil(Regne *r,int socket,Trama *t){

    int indexRuta=-1;
    int n_paraules = 0;
    char **paraules = tokenizer(t->nom_origen, &n_paraules, ':');//ip:port    for (int i=0;i<r->n_rutes;i++){
    for(int i=0;i<r->n_rutes;i++){
        if(r->rutes[i].portDest==atoi(paraules[1])){
            indexRuta=i;
            break;
        }
    }
    if(indexRuta<0){
        freeTokens(paraules, n_paraules);
        return;
    }

    char *missDir; 
    asprintf(&missDir, ".%s", r->rutaArxius); 
    char *fitxerPath;
    asprintf(&fitxerPath, "%s/%s_sigil.png",missDir,r->rutes[indexRuta].nomDest);
    int fd = open(fitxerPath, O_WRONLY | O_CREAT, 0666);

    if (fd < 0) {
        printString(1, "Error opening sigil file.\n");
        free(fitxerPath);
        free(missDir);    
        return;
    }

    key_t semKey = generarClauIPC(r->rutes[indexRuta].nomDest, r->nom, 1);
        semaphore semPledge;
        SEM_constructor_with_name(&semPledge, semKey);
  

    
    lseek(fd, 0, SEEK_END);
    write(fd, t->data, t->dataLength);
    //semafor signal aqui per indicar que envii la seguent trama
    SEM_signal(&semPledge); 
    if(r->rutes[indexRuta].sigilSize <= getSize(fd)){ 
        char data[DATA_SIZE];
        int size=getSize(fd);
        char *md5 = calcularMD5SUM(fitxerPath);
        if (!md5) {
            printString(1, "Error calculant MD5\n");
            close(fd);
            free(fitxerPath);
            free(missDir);
            freeTokens(paraules, n_paraules);
            return;
        }
        
        Trama * ackMD5=crearTrama(0x32, NULL, 0,NULL, strlen("CHECK_OK"),"CHECK_OK");
        if(strcmp(md5, r->rutes[indexRuta].MD5SUM)==0){
            enviarTramaDirecte(ackMD5, atoi(paraules[1]), paraules[0], r->rutes[indexRuta].nomDest);
             r->rutes[indexRuta].ackMD5SUMStatus=1;
        }else{
            ackMD5->dataLength=strlen("CHECK_KO");
            memcpy(ackMD5->data,"CHECK_KO",strlen("CHECK_KO"));
            enviarTramaDirecte(ackMD5, atoi(paraules[1]), paraules[0], r->rutes[indexRuta].nomDest);
            r->rutes[indexRuta].ackMD5SUMStatus=0;
        }
        free(md5);


    }
    close(fd);
    free(fitxerPath);
    free(missDir); 
    freeTokens(paraules, n_paraules);

}

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
            r->rutes[i].alianca = 'O';
            if (r->rutes[i].ipDest) {
                free(r->rutes[i].ipDest);
            }
            r->rutes[i].ipDest =strdup(paraules2[0]);
            r->rutes[i].portDest = atoi(paraules2[1]);
            r->rutes[i].sigilSize = atoi(paraules[2]);
            r->rutes[i].MD5SUM[0] = '\0';
            strncpy(r->rutes[i].MD5SUM, paraules[3], 33 - 1);
            r->rutes[i].MD5SUM[33 - 1] = '\0';
            trobat=1;
            r->rutes[i].ackFitxerStatus=1;
            printString(1, ">>> Alliance request received from ");
            printString(1, paraules[0]);
            printString(1, ".\n");
            break;
        }
    }    
    if(trobat){
        Trama * ackTrama = crearTrama(0x31, NULL, 0, NULL, 2, "OK");
        enviarTramaDirecte(ackTrama, atoi(paraules2[1]), paraules2[0], paraules[0]);//es fa el frre de trama aqui

        //afegirAck(&r->rutes[trobat],0 , 0, "pledge", paraules[0]);//tipus 0 = ackfitxer

    }
    freeTokens(paraules, n_paraules);
    freeTokens(paraules2, n_paraules2);
}

 void processarTramaPledgeRespond(Trama *t, Regne *r,int client_sock) {

    
   
    int n_paraules = 0;
    int n_paraules2 = 0;
    int currRuta=0;
    char **paraules = tokenizer(t->nom_origen, &n_paraules, ':');
    char **paraules2 = tokenizer(t->data, &n_paraules2, '&');


    if (n_paraules != 2 || n_paraules2 != 2) {
        freeTokens(paraules, n_paraules);
        freeTokens(paraules2, n_paraules2);
        printString(1, "Pledge respond trama malformed\n");
        return;
    }

    if (strcasecmp(paraules2[0], "ACCEPT") == 0) {
        for (int i = 0; i < r->n_rutes; i++) {
            if (strcmp(r->rutes[i].nomDest,paraules2[1] ) == 0) {
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
    } else if (strcasecmp(paraules2[0], "REJECT") == 0) {
        for (int i = 0; i < r->n_rutes; i++) {
            if (strcmp(r->rutes[i].nomDest, paraules2[1]) == 0) {
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
        write(client_sock, nackTrama, sizeof(Trama));
        free(nackTrama);

    }else if(r->rutes[currRuta].alianca=='S'){
        Trama * ackTrama = crearTrama(0x31, NULL, 0, NULL, 2, "OK");
        write(client_sock, ackTrama, sizeof(Trama));
        free(ackTrama);
    }
   // enviarTramaACKFitxer(paraules[0], atoi(paraules[1]), t->nom_desti, "OK",client_sock);
    freeTokens(paraules, n_paraules);
    freeTokens(paraules2, n_paraules2);
}


void processarTramaLlistarProductes(Trama *t, Regne *r,int client_sock) {

       printString(1, "LIST PRODUCTS request from ");
        printString(1, t->nom_origen);
        int indexAliat=-1;
        for(int i=0;i<r->n_rutes;i++){
            if(strcmp(r->rutes[i].nomDest,t->data)==0){
                if(r->rutes[i].alianca!='S'){
                        Trama * authtrama = crearTrama(0x25, r->ip, r->port, t->data, 5, "AUTH");
                        enviarTramaSocket(authtrama, client_sock);
                        free(authtrama);
                        printString(1, "You are not allied with this Realm or it is inactive. Sending AUTH NACK.\n");
                    return;
                }
                if(r->rutes[i].alianca=='S'){
                    indexAliat=i;
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
        char *md5 = calcularMD5SUM(filename);
        if (!md5) {
            printString(1, "Error calculant MD5\n");
            close(fd);
            free(filename);
            return;
        }
        snprintf(data, DATA_SIZE, "%s&%d&%s",filename,size,md5);
        free(md5);
        free(filename);
        Trama * trama = crearTrama(0x12, r->ip, r->port, r->nom , strlen(data),data);
        write(client_sock, trama, sizeof(Trama));
        printString(1, "Products delivered.\n");

        Trama * acktrama=crearTrama(0, NULL, 0, NULL, 0, NULL);
        int n=read(client_sock, acktrama, sizeof(Trama));
        if(n<=0){
            free(acktrama);
            printString(1, "Did not receive ACK/NACK for product list.\n");
            close(fd);
            free(trama);
            return;
        }
        if (acktrama->tipus == 0x31) {  

            key_t semKey = generarClauIPC(r->nom, r->rutes[indexAliat].nomDest, 2);   
            semaphore semSendProducts;
            SEM_constructor_with_name(&semSendProducts, semKey);
            SEM_init(&semSendProducts, 1);
            size_t startPos = 0;
            char *blockData = NULL;
            Trama *productTrama = NULL;
            off_t size = lseek(fd, 0, SEEK_END);
            lseek(fd, 0, SEEK_SET);

                while (startPos < size) {
                    SEM_wait(&semSendProducts);  
                    if (r->exit && *(r->exit)) {
                        close(fd);
                        printString(1, "Send aborted by exit flag.\n");
                        return NULL;
                    }
                    size_t chunk = 275;  
                    if (startPos + chunk > size)
                        chunk = size - startPos;
                    blockData = readFd(fd, startPos, chunk);
                    if (!blockData) {
                        printString(1, "Error reading product file\n");
                        close(fd);
                        return NULL;
                    }
                    productTrama = crearTrama(0x13, r->ip, r->port, r->nom, chunk, blockData);
                    if (!productTrama) {
                        printString(1, "Error creating product trama\n");
                        free(blockData);
                        close(fd);
                        return NULL;
                    }
                    enviarTramaSocket(productTrama, client_sock);
                    
                    free(blockData);
                    free(productTrama);
                    startPos += chunk;
                }
                printString(1, "All products sent.\n");
            }
        free(acktrama);
        free(trama);
        close(fd);
        
}

/*

AQUI ES PROCESSEN TOTES LES TRAMES A PARTIR DEL TIPUS

*/
 void processarTrama(Trama *t, Regne *r,int client_sock) {

    switch(t->tipus){

        case 0x01: 
                processarTramaPledge(t, r, client_sock); 
                break;
        
            case 0x02:
                rebreSigil(r,client_sock,t);
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
                /*for (int i =0;i<r->n_envoys;i++){
                    //mirar el nom del regne del envoy
                }*/
                if(t->data!=NULL && strcmp(t->data,"OK")==0){
                    r->envoys[0].ackFitxer=1;
                    printString(1, "Trama ack fitxer  OK rebuda:\n");
                }else{
                    r->envoys[0].ackFitxer=2;
                    printString(1, "Trama ack fitxer  KO rebuda:\n");

                }
                
                break;
            case 0x32:
                 if(t->data!=NULL && strcmp(t->data,"CHECK_OK")==0){
                    r->envoys[0].ackMD5SUM=1;
                    printString(1, "Trama MD5 OK rebuda:\n");
                }else{
                    r->envoys[0].ackMD5SUM=2;
                    printString(1, "Trama MD5 KO rebuda:\n");

                }
             
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


void creaServer(Regne *r){
    r->fd_server = create_server_socket(r->port);
    if (r->fd_server < 0) {
        printString(1, "Error creating server socket\n");
    }
}

void* runServer(void *arg) {
    ThreadUnServidor *tus = (ThreadUnServidor *)arg;
    Regne *r = tus->r;
    ServerThread *st = tus->st;
    free(tus);
    Trama* t = crearTrama(0, "", 0, "", 0, "");
    ssize_t servRead = read(st->client_fd, t, sizeof(Trama));
    if (servRead <= 0) {
        //printString(1, "Connexió tancada o error.\n");
        close(st->client_fd);
        free(t);
        st->thread_complete = 1;
        return NULL;
    }
    if (servRead != sizeof(Trama)) {
        printString(1, "Trama rebuda incompleta.\n");
        free(t);
        st->thread_complete = 1;
        return NULL;
    }

    if (tocaIgnorarTrama(t->tipus)) {
        processarTrama(t, r, st->client_fd);
        free(t);
        st->thread_complete = 1;
        return NULL;
    }
    if (!comprovarChecksum(t) && t->checksum > 0) {
        int n_paraules = 0;
        char **paraules = tokenizer(t->nom_origen, &n_paraules, ':');
        Trama *tramaNACK = crearTrama(0x69, NULL, 0, NULL, 0, NULL);
        enviarTramaDirecte(tramaNACK, atoi(paraules[1]), paraules[0], t->nom_desti);
        freeTokens(paraules, n_paraules);
        free(t);
        st->thread_complete = 1;
        return NULL;
    }
    if (strcmp(t->nom_desti, r->nom) == 0) {
        processarTrama(t, r, st->client_fd);
        free(t);
    } else {
        enviarTramaDinamica(t, r, 1); // s'allibera dins
    }
    st->thread_complete = 1;
    return NULL;
}

void closeServer(Regne *r){//per cntol +c
    if (r->fd_server > 0) {
     close(r->fd_server);
    }
}
