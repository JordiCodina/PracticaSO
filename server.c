

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
       // printString(1,ERROR_ACCEPT);
        
        return -1;
    }
    
    return new_socket;
}

/*

@brief Aixo es per eliminar el thread de la llista de threads del servidor 

*/
void removeThreadServidor(Regne *r, int index) {
    if (r->n_serverThreads <= 0 || index < 0 || index >= r->n_serverThreads) return;

    free(r->st[index]); 
    for (int i = index; i < r->n_serverThreads - 1; i++) {
        r->st[i] = r->st[i + 1];
    }
    r->n_serverThreads--;
    if (r->n_serverThreads == 0) {
        free(r->st);
        r->st = NULL;
    } else {
        r->st = realloc(r->st, r->n_serverThreads * sizeof(ServerThread*));
    }
}

/*
@brief Fa join als threads del servidor que han acabat amb  thread_complete==1

*/
void joinThreadServidor(Regne *r) {
     for(int i=0;i<r->n_serverThreads;){
        if(r->st[i]->thread_complete==THREAD_DONE){
            close(r->st[i]->client_fd);
            pthread_join(r->st[i]->thread_id, NULL);
            removeThreadServidor(r, i); 
        } else {
            i++; 
        }
    }
}
/*
@brief Afegeix un nou ServerThread a la llista de serverThreads del regne.

*/
void afegirthreadServidor(Regne *r, ServerThread *st) {
    if (r->st == NULL) {
        r->st = malloc(sizeof(ServerThread*));
        r->n_serverThreads = 1;
    } else {
        r->st = realloc(r->st, (r->n_serverThreads + 1) * sizeof(ServerThread*));
        r->n_serverThreads++;
    }
    r->st[r->n_serverThreads - 1] = st; 
}

/*

@brief aqui es verifica si el fitxer de la llista de comandes es correcte(suficient stock o que exsiteix tots els productes) i aplica els canvis a l'stock
@param fdCommandes descriptor del fitxer de comandes que l'hem rebut i l'hem posat a la nostre carpeta
@param fdStock descriptor del fitxer d'stock que te 
@param r Punter a la estructura Regne que conté la informació del regne actual.

*/
int processarComandaProductesFitxer(int fdCommandes, int fdStock, Regne *r) {

    //llegir fitxer comandes i actualitzar stock
    int size = 0;
    lseek(fdCommandes, 0, SEEK_SET);
    int n_productes = 0;
    //carregar productes del stock que tens a la carpeta de cada regne
    Producte *productes = extreureProductes(fdStock, &n_productes);
    while (1) {
        //llegir linia a linia
        char *data = readUntil(fdCommandes, '-', &size);
        if (data == NULL) {
            break;
        }

        int n_paraules = 0;
        char **paraules = tokenizer(data, &n_paraules, ':');
        if (n_paraules == 2) {
            int amount = atoi(paraules[1]);
            //aquesta funcio treu automaticament els productes i retorna el status
            int result = treureDelStock(productes, &n_productes,paraules[0], amount);
            char *miss3;
            asprintf(&miss3,"stock%s.db", r->nom);
            actualitzarProductes(miss3, productes, n_productes);
            //printString(1, "Stock updated successfully.\n");
            free(miss3);
            freeTokens(paraules, n_paraules);
            free(data);
            //es comprova els 3 casos que poden haver , 1 es que ha anat b
            switch (result) {
                case 0:
                    free(productes);
                    return 0;
                case 1:
                    break;
                default:
                    free(productes);
                    return -1;
            }
        } else {
            freeTokens(paraules, n_paraules);
            free(data);
        }
    }
    free(productes);
    return 1;
}


/*

@brief Funció que gestiona inicialment la recepció de la comanda de productes (trama 0x14),mante el socket 
i per tant tambe fa de comunicador  amb el mateix socket entre el regne que envia la llista de compra.
Per tant no nomes rep 0x14 que es la trama inicial per rebre la comanda de productes, sino que tambe rep les trames 0x15
que es per rebre el fitxer de comandes i  tambe enviar les acks corresponents.

Finalment tambe actualitza l'stock si tot ha anat be enviant la confirmacio al regne aliat.

@param t Punter a la estructura Trama que conté la informació de la trama rebuda
@param r Punter a la estructura Regne que conté la informació del regne actual.
@param client_sock Descriptor del socket del client que ha enviat la trama.

*/
void rebreComandaProductes (Trama *t, Regne *r, int client_sock){
   
    int n_paraules = 0;
    char **paraules=tokenizer(t->data, &n_paraules, '&');
    if(n_paraules!=3){
        printString(1, "Error: trama 0x14 mal formada.\n");
        freeTokens(paraules, n_paraules);
        return;
    }
    int indexAliat=-1;
    int n_paraules2=0;
    char **paraules2=tokenizer(t->nom_origen, &n_paraules2, ':');//ip:port
    if(n_paraules2!=2){
        printString(1, "Error: trama 0x14 mal formada (nom_origen).\n");
        freeTokens(paraules, n_paraules);
        freeTokens(paraules2, n_paraules2);
        return;
    }
    //buscar index aliat
    for(int i=0;i<r->n_rutes;i++){
        if(r->rutes[i].portDest==atoi(paraules2[1])){
            indexAliat=i;
            break;
        }
    }
    //
    printString(1, ">>> Trade request received from Dragonstone.\n");
    char ackdata[DATA_SIZE];
    snprintf(ackdata, DATA_SIZE, "OK&%s",r->nom);
    //si es correcte enviar ack Fitxer
    Trama * ackTrama=crearTrama(0x31, NULL, 0, NULL, strlen(ackdata), ackdata);
    enviarTramaSocket(ackTrama, client_sock);
    

    free(ackTrama);

    char *nomFitxer = crearRutaComanda(r->rutaArxius, r->nom, r->rutes[indexAliat].nomDest);
    int fdCommandes = crearFitxerComandes(nomFitxer);
    if(fdCommandes==-1){
        printString(1, "Error creating trade file to receive commands\n");
         free(nomFitxer);
         freeTokens(paraules, n_paraules);
        freeTokens(paraules2, n_paraules2);
        return;
    }
        int currentSize=0;
        int size=atoi(paraules[1]);
        //rebre fitxer comandes i escriure al fitxer de la carpeta del regne
        while(currentSize<size){
            usleep(1000); 
            if (r->exit && *(r->exit)) {
                    close(fdCommandes);
                    free(nomFitxer);
                    freeTokens(paraules, n_paraules);
                    freeTokens(paraules2, n_paraules2);
                    return;
            }
            Trama tramaRebuda;
            int n = read(client_sock, &tramaRebuda, sizeof(Trama));
            if (n <= 0) {
                printString(1, "Connection closed before shopping list was complete.\n");
                 freeTokens(paraules, n_paraules);
                 freeTokens(paraules2, n_paraules2);
                return;
            }
            if (tramaRebuda.tipus != 0x15) {
                printString(1, "Trama is incorrect\n");
                 freeTokens(paraules, n_paraules);
                freeTokens(paraules2, n_paraules2);
                return;
            }
            
            write(fdCommandes, tramaRebuda.data, tramaRebuda.dataLength);
            currentSize+=tramaRebuda.dataLength;

        }
        
       

        close(fdCommandes);
        //tornem a obrir el fitxer de comandes per llegir-lo i processar la comanda
        fdCommandes = open(nomFitxer, O_RDONLY);
        if (fdCommandes == -1) {
            printString(1, "Error opening trade file for reading\n");
            free(nomFitxer);
            return;
        }
        char *md5 = calcularMD5SUM(nomFitxer);
            if (!md5) {
                printString(1, "Error calculating MD5 for received trade file\n");
                close(fdCommandes);
                free(nomFitxer);
                freeTokens(paraules, n_paraules); 
                freeTokens(paraules2, n_paraules2);
                return;
            }
            if (strcmp(md5, paraules[2]) != 0) {
                Trama *md5nAck = crearTrama(0x32, NULL, 0, NULL, strlen("CHECK_KO"), "CHECK_KO");
                enviarTramaSocket(md5nAck, client_sock);
                free(md5nAck);
                printString(1, "MD5 checksum does not match for received trade file\n");
                free(md5);
                close(fdCommandes);
                free(nomFitxer);
                freeTokens(paraules, n_paraules); 
                freeTokens(paraules2, n_paraules2);
                return;
            }

            free(md5);
            freeTokens(paraules, n_paraules); 
            freeTokens(paraules2, n_paraules2);
        //ack md5 correcte
        char *datamdACK;
        asprintf(&datamdACK, "CHECK_OK&%s", r->nom);
        Trama *md5Ack = crearTrama(0x32, NULL, 0, NULL, strlen(datamdACK), datamdACK);
        enviarTramaSocket(md5Ack, client_sock);
        free(md5Ack);
        free(datamdACK);
        free(nomFitxer);

        //processar comanda i actualitzar stock
        char *stockFile;
        asprintf(&stockFile, "stock%s.db",r->nom);
        int fdStock = open(stockFile, O_RDWR);
        if(fdStock==-1){
            printString(1, "Error opening stock file to read/write\n");
            close(fdCommandes);
            free(stockFile);
           
            return;
        }

        free(stockFile);
        //
        int comandaStatus=processarComandaProductesFitxer(fdCommandes, fdStock, r);
         close(fdCommandes);
            close(fdStock);
            //modificar trama->data
          char dataRes[DATA_SIZE];
        if(comandaStatus==1){//OK
            snprintf(dataRes, DATA_SIZE, "OK");
            printString(1, ">>> Order processed successfully. Stock updated.\n");
        }else if(comandaStatus==0){//REJECT&OUT_OF_STOCK
            snprintf(dataRes, DATA_SIZE, "REJECT&OUT_OF_STOCK");
            printString(1, ">>> Order rejected. Insufficient stock.\n");
        }else if(comandaStatus==-1){//REJECT&UNKNOWN_PRODUCT
            snprintf(dataRes, DATA_SIZE, "REJECT&UNKNOWN_PRODUCT");
            printString(1, ">>> Order rejected. Unknown product in order.\n");
        }
    Trama * tramaRespostaComanda=crearTrama(0x16, r->ip, r->port, r->rutes[indexAliat].nomDest, strlen(dataRes), dataRes);
    enviarTramaSocket(tramaRespostaComanda, client_sock);
    free(tramaRespostaComanda);


} 

//marcar alianca establerta ON pending
void marcarAliancaPendent(Regne *r, int indexRuta){ 
        r->rutes[indexRuta].ackMD5SUMStatus=1;
            r->rutes[indexRuta].alianca = 'O';
            printString(1, ">>> Alliance request received from ");
            printString(1, r->rutes[indexRuta].nomDest);
            printString(1, ".\n");
}
/*

RESPONDRE A PETCIONS DEL CLIENT COM A SERVIDOR

*/
/*
@brief Funció que gestiona la recepció del fitxer de productes (trama 0x13) des d'un regne aliat.
Rep una a una  en threads de servidor diferents i finalment si tot ha anat be envia l'ack corresponent.

@param r Punter a la estructura Regne que conté la informació del regne actual.
@param t Punter a la estructura Trama que conté la informació de la trama rebuda.

*/
void rebreSigil(Regne *r,Trama *t){
  // printf("Writing sigil data ...\n");
    
    int indexRuta=-1;
    int n_paraules = 0;
    char **paraules = tokenizer(t->nom_origen, &n_paraules, ':');//ip:port    for (int i=0;i<r->n_rutes;i++){
    for(int i=0;i<r->n_rutes;i++){
        if(r->rutes[i].portDest==atoi(paraules[1]) && strcmp(r->rutes[i].nomDest, "DEFAULT")!=0){
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
    asprintf(&fitxerPath, "%s/%s_sigilAliat.png",missDir,r->rutes[indexRuta].nomDest);
    int fd = open(fitxerPath, O_WRONLY | O_CREAT, 0666);
    if (fd < 0) {
        printString(1, "Error opening sigil file.\n");
        free(fitxerPath);
        free(missDir);    
        return;
    }
     //si arriba un trama  ens posem al final del fitxer de sigil del regne desti i escrivim
    lseek(fd, 0, SEEK_END);
    write(fd, t->data, t->dataLength);
    //printf("Sigil size: %d, File size: %d\n", r->rutes[indexRuta].sigilSize, getSize(fd));

    //comprovar si ja hem rebut tot el fitxer , si es aixi comprovar md5 i enviar ack corresponent
    if(r->rutes[indexRuta].sigilSize <= getSize(fd)){ 

         //comrpovar md5 i enviar ack corresponent
        char *md5 = calcularMD5SUM(fitxerPath);
        if (!md5) {
            printString(1, "Error calculant MD5\n");
            close(fd);
            free(fitxerPath);
            free(missDir);
            freeTokens(paraules, n_paraules);
            return;
        }
        
       
         char *dataackMD5;
        asprintf(&dataackMD5, "CHECK_OK&%s", r->nom);
        Trama * ackMD5=crearTrama(0x32, NULL, 0,NULL, strlen(dataackMD5),dataackMD5);
        free(dataackMD5);
        if(strcmp(md5, r->rutes[indexRuta].MD5SUM)==0){
            enviarTramaDirecte(ackMD5, atoi(paraules[1]), paraules[0], r->rutes[indexRuta].nomDest);
            marcarAliancaPendent(r, indexRuta);


        }else{
            char *dataackMD5;
            asprintf(&dataackMD5, "CHECK_KO&%s", r->nom);
            ackMD5->dataLength=strlen(dataackMD5);
            memcpy(ackMD5->data,dataackMD5,strlen(dataackMD5));
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



/*

    rebre PLEDGE
    @brief Funció que gestiona la recepció de la petició d'aliança (trama 0x02) des d'un regne aliat.
    Tambe crea el fitxer del segell aliat buit i envia l'ack corresponent.

@param t Punter a la estructura Trama que conté la informació de la trama rebuda
@param r Punter a la estructura Regne que conté la informació del regne actual.
@param socket Descriptor del socket del client que ha enviat la trama.

*/

 void processarTramaPledge(Trama *t, Regne *r,int socket) {
    int n_paraules = 0;
    int n_paraules2 = 0;
    int trobat=0;
    int indexRuta=-1;
    char **paraules  = tokenizer(t->data, &n_paraules, '&');
    char **paraules2 = tokenizer(t->nom_origen, &n_paraules2, ':');//ip:port
    if (n_paraules != 4) {
        freeTokens(paraules, n_paraules);
        return;
    }
    for (int i = 0; i < r->n_rutes; i++) {
        if (strcmp(r->rutes[i].nomDest, paraules[0]) == 0) {
            //r->rutes[i].alianca = 'O';
            if (r->rutes[i].ipDest) {
                free(r->rutes[i].ipDest);
            }
            indexRuta=i;
            r->rutes[i].ipDest =strdup(paraules2[0]);
            r->rutes[i].portDest = atoi(paraules2[1]);
            r->rutes[i].sigilSize = atoi(paraules[2]);
            r->rutes[i].MD5SUM[0] = '\0';
            strncpy(r->rutes[i].MD5SUM, paraules[3], 33 - 1);
            r->rutes[i].MD5SUM[33 - 1] = '\0';
            trobat=1;
            r->rutes[indexRuta].ackFitxerStatus=1;
           
            break;
        }
    }    
    if(trobat){
        crearDirectori(r->rutaArxius);
        char *pathPNG;
        asprintf(&pathPNG, ".%s/%s_sigilAliat.png", r->rutaArxius, r->rutes[indexRuta].nomDest);
        int fdsigil = open(pathPNG, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fdsigil < 0) {
            free(pathPNG);
            close(socket);
            return;
        }
        close(fdsigil); // tancar ara, després obriràs per escriure trames
        free(pathPNG);
        
        char ackdata[DATA_SIZE];
        snprintf(ackdata, DATA_SIZE, "OK&%s",r->nom);
        Trama * ackTrama = crearTrama(0x31, NULL, 0, NULL, strlen(ackdata), ackdata);
        enviarTramaDirecte(ackTrama, atoi(paraules2[1]), paraules2[0], paraules[0]);//es fa el frre de trama aqui
    }
    freeTokens(paraules, n_paraules);
    freeTokens(paraules2, n_paraules2);
}

/*

Rebre PLEDGE RESPOND

*/
 void processarTramaPledgeRespond(Trama *t, Regne *r) {
   
    int n_paraules = 0;
    int n_paraules2 = 0;
    char **paraules = tokenizer(t->nom_origen, &n_paraules, ':');
    char **paraules2 = tokenizer(t->data, &n_paraules2, '&');

    if (n_paraules != 2 || n_paraules2 != 2) {
        freeTokens(paraules, n_paraules);
        freeTokens(paraules2, n_paraules2);
        printString(1, "Pledge respond trama malformed\n");
        return;
    }
    int indexEnvoy = trobarEnvoyIndex(r, paraules2[1]);
       if(indexEnvoy==-1){
            printString(1, "No envoy found for the specified realm in pledge respond.\n");
            freeTokens(paraules, n_paraules);
            freeTokens(paraules2, n_paraules2);
            return;
    }
    if (strcasecmp(paraules2[0], "ACCEPT") == 0) {

        ForkRegneWritesEnvoy *frwe= crearForkRegneWritesEnvoy(5,paraules2[1], NULL , indexEnvoy, 0, 1, paraules[0], atoi(paraules[1]));
        write(r->envoys[indexEnvoy].pipeWrite,frwe, sizeof(ForkRegneWritesEnvoy));
        free(frwe);
        
    } else if (strcasecmp(paraules2[0], "REJECT") == 0) {

        ForkRegneWritesEnvoy *frwe = crearForkRegneWritesEnvoy(5,paraules2[1], NULL , indexEnvoy, 0, 2, paraules[0], atoi(paraules[1]));
        write(r->envoys[indexEnvoy].pipeWrite,frwe, sizeof(ForkRegneWritesEnvoy));
        free(frwe);
    }
    freeTokens(paraules, n_paraules);
    freeTokens(paraules2, n_paraules2);
}

/*

rebre LLIST PRODUCTS

@brief Funció que gestiona la recepció de la petició de llista de productes (trama 0x11) des d'un regne aliat.
Rep la petició, comprova si hi ha alianca i si es així envia la llista de productes.
@param t Punter a la estructura Trama que conté la informació de la trama rebuda
@param r Punter a la estructura Regne que conté la informació del regne actual.
@param client_sock Descriptor del socket del client que ha enviat la trama.


*/

void processarTramaLlistarProductes(Trama *t, Regne *r,int client_sock) {


        int indexAliat=-1;
        for(int i=0;i<r->n_rutes;i++){
            char *ipPort;
            asprintf(&ipPort, "%s:%d", r->rutes[i].ipDest, r->rutes[i].portDest);
            if(strcmp(ipPort,t->nom_origen)==0 && strcmp(r->rutes[i].nomDest,"DEFAULT")!=0){
                
                if(r->rutes[i].alianca!='S'){
                        Trama * authtrama = crearTrama(0x25, r->ip, r->port, t->data, 5, "AUTH");
                        enviarTramaSocket(authtrama, client_sock);
                        free(authtrama);
                        printString(1, "You are not allied with this Realm or it is inactive. Sending AUTH NACK.\n");
                        free(ipPort);
                    return;
                }
                if(r->rutes[i].alianca=='S'){
                    
                    printString(1, "LIST PRODUCTS request from ");
                    printString(1, r->rutes[i].nomDest);
                    printString(1, ".\n");
                    indexAliat=i;
                    
                }
                free(ipPort);
                break;
            }
            free(ipPort);
            
        }
        if(indexAliat==-1){
            printString(1, "Realm not found in routes.\n");
            return;
        }
        printString(1,"Sending product list.\n");
        char * filename;
        asprintf(&filename, "stock%s.db",r->nom);
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
  

        Trama * trama = crearTrama(0x12, r->ip, r->port, r->rutes[indexAliat].nomDest , strlen(data),data);
        enviarTramaSocket(trama, client_sock);
        free(trama);
        printString(1, "Products delivered.\n");

        Trama * acktrama=crearTrama(0, NULL, 0, NULL, 0, NULL);
        int n=read(client_sock, acktrama, sizeof(Trama));
        if(n<=0){
            free(acktrama);
            printString(1, "Did not receive ACK/NACK for product list.\n");
            close(fd);
            
            return;
        }

        if (acktrama->tipus == 0x31) {  
            
            int startPos = 0;
            char *blockData = NULL;
            Trama *productTrama = NULL;
            int size = lseek(fd, 0, SEEK_END);
            lseek(fd, 0, SEEK_SET);

                while (startPos < size) {
                    
                    if (r->exit && *(r->exit)) {
                        close(fd);
                        printString(1, "Send aborted by exit flag.\n");
                        return;
                    }
                    int chunk = 275;  
                    if (startPos + chunk > size)
                        chunk = size - startPos;
                    blockData = readFd(fd, startPos, chunk);
                    if (!blockData) {
                        printString(1, "Error reading product file\n");
                        close(fd);
                        return;
                    }
                    productTrama = crearTrama(0x13, r->ip, r->port, r->nom, chunk, blockData);
                    if (!productTrama) {
                        printString(1, "Error creating product trama\n");
                        free(blockData);
                        close(fd);
                        return;
                    }
                    enviarTramaSocket(productTrama, client_sock);
                    
                    free(blockData);
                    free(productTrama);
                    startPos += chunk;
                }
                printString(1, "All products sent.\n");
            }
        free(acktrama);
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
                rebreSigil(r,t);
                break;
            case 0x03:
                processarTramaPledgeRespond(t, r);
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

                rebreComandaProductes(t, r, client_sock);
               
                break;
            case 0x15:

                printString(1, "Trama enviament de fitxer productes rebuda:\n");
              
                break;
            case 0x16:
                printString(1, "Trama  rebuda:\n");
              
                break;
            case 0x27:// trama que indica que s'ha de finalitzat un realm aliat i has de posar la ruta com a inactiva
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
                //Ping pong
                Trama *ping=crearTrama(0x26, r->ip, r->port, r->nom, 4, "PONG");
                enviarTramaSocket(ping, client_sock);
                free(ping);

                
                break;
            case 0x31:
                int statusACKFitxer=0;
                int n_paraules = 0;
                char **paraules = tokenizer(t->data, &n_paraules, '&');
                if(strcmp("OK",paraules[0])==0){
                    statusACKFitxer=1;
                    printString(1, "Trama ack fitxer  OK rebuda:\n");
                }else{
                    statusACKFitxer=2;
                    printString(1, "Trama ack fitxer  KO rebuda:\n");
                }

                int quinEnvoy=-1;                
                for(int i=0;i<r->n_envoys;i++){
                    if(r->envoys[i].nomRegneViatge==NULL){
                        continue;
                    }

                    if(strcmp(r->envoys[i].nomRegneViatge,paraules[1])==0){
                        quinEnvoy=i;
                        break;
                    }
                }
                if(quinEnvoy!=-1){
                    //r->envoys[quinEnvoy].ackFitxer=statusACKFitxer;
                    ForkRegneWritesEnvoy *frwe = crearForkRegneWritesEnvoy(3, NULL, NULL, quinEnvoy, statusACKFitxer, 0, NULL, 0);
                    write(r->envoys[quinEnvoy].pipeWrite, frwe, sizeof(ForkRegneWritesEnvoy));
                    free(frwe);
                }
                
                freeTokens(paraules, n_paraules);

                break;
            case 0x32:
                int statusACKMD5=0;
                int n_paraules2 = 0;
                char **paraules2 = tokenizer(t->data, &n_paraules2, '&');
                 if(strcmp(paraules2[0],"CHECK_OK")==0){
                    statusACKMD5=1;
                    printString(1, "Trama MD5 OK rebuda:\n");
                }else{
                    printString(1,"ERROR: MD5 sum is incorrect. Try again\n");
                }
                
                int quinEnvoy2=-1;
                for(int i=0;i<r->n_envoys;i++){
                    
                    if(r->envoys[i].nomRegneViatge==NULL){
                        continue;
                    }
                    if(strcmp(r->envoys[i].nomRegneViatge,paraules2[1])==0){
                        quinEnvoy2=i;
                        break;
                    }
                }
                if(quinEnvoy2!=-1){
                    //r->envoys[quinEnvoy2].ackMD5SUM=statusACKMD5;
                   /* frwe->comanda = 4; // enviar ackMD5
                    frwe->indexEnvoy = quinEnvoy2;
                    frwe->ackStatus =statusACKMD5;*/
                    ForkRegneWritesEnvoy *frwe = crearForkRegneWritesEnvoy(4, NULL, NULL, quinEnvoy2, statusACKMD5, 0, NULL, 0);
                    write(r->envoys[quinEnvoy2].pipeWrite, frwe, sizeof(ForkRegneWritesEnvoy));
                    free(frwe);
                }else{
                    printString(1, "No matching envoy found for MD5 ACK\n");
                }
                freeTokens(paraules2, n_paraules2);
             
                break;
            case 0x69:
                //printString(1, "Trama error : NACK  rebuda:\n");
                
                break;           
            default:
                printString(1, "Tipus de trama desconegut.\n");
                break;
        }
    }

/*

@brief Funció que comprova si la trama és especial (com ara ACK, NACK, etc.) basant-se en el tipus de trama.
@param tipus Enter que representa el tipus de trama.
@return Retorna 1 si la trama és especial, 0 en cas contrari.

*/
int esTramaEspecial(int tipus){
    if(tipus==0x69 || tipus==0x31 || tipus==0x32 || tipus==0x03){//poden haver-hi mes
        return 1;
    }
    return 0;
}
/*
@brief Funció que comprova el checksum d'una trama.
@param t Punter a la estructura Trama que conté la informació de la trama.
@return Retorna 1 si el checksum és correcte, 0 en cas contrari.
*/
int comprovarChecksum(Trama *t){
    unsigned short checksumCalculat = calcularChecksum(t);
    if (checksumCalculat != t->checksum) {
        return 0;
    }
    return 1;
}

/*
@brief Funció que comprova si la trama indica que s'ha de finalitzar el servidor.
@param tipus Enter que representa el tipus de trama.
@return Retorna 1 si s'ha de finalitzar el servidor, 0 en cas contrari.
*/
int tocaFinalitzarServidor(int tipus){
    if(tipus==0x27){
        return 1;
    }
    return 0;
}


/*
@brief Funció que crea el socket del servidor i l'associa al port especificat a la estructura Regne.
@param r Punter a la estructura Regne que conté la informació del regne actual.
*/

void creaServer(Regne *r){
    r->fd_server = create_server_socket(r->port);
    if (r->fd_server < 0) {
        printString(1, "Error creating server socket\n");
    }
}

/*
@brief Funció que gestiona la comunicació amb un client en un thread de servidor.Ve del select a terminal.c o start trade
@param arg Punter a la estructura ThreadUnServidor que conté la informació del regne i del thread del servidor.
*/
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
        st->thread_complete = THREAD_DONE;
        return NULL;
    }
    if (servRead != sizeof(Trama)) {
        printString(1, "Trama rebuda incompleta.\n");
        free(t);
        st->thread_complete = THREAD_DONE;
        return NULL;
    }

    if (esTramaEspecial(t->tipus)) {//trames com acks que si no posem aixo mai trobarien un match del nomDest ja que  els camps estan buits
        processarTrama(t, r, st->client_fd);
        free(t);
        st->thread_complete = THREAD_DONE;
        return NULL;
    }
    if (!comprovarChecksum(t) && t->checksum > 0) {
        int n_paraules = 0;
        char **paraules = tokenizer(t->nom_origen, &n_paraules, ':');
        Trama *tramaNACK = crearTrama(0x69, NULL, 0, NULL, 0, NULL);
        enviarTramaDirecte(tramaNACK, atoi(paraules[1]), paraules[0], t->nom_desti);
        freeTokens(paraules, n_paraules);
        free(t);
        st->thread_complete = THREAD_DONE;
        return NULL;
    }
    if (strcmp(t->nom_desti, r->nom) == 0) {// Trama per aquest regne
        processarTrama(t, r, st->client_fd);
        free(t);
    } else {
        // Forwarding trama(no es per aquest regne , seguent hop)
        char *nom_origen = strdup(t->nom_origen); 
        char *nom_desti = strdup(t->nom_desti);
        int status = -1;
        if(t->tipus==0x02){//aixo es per que no faci el printf del forwarding 
            status = enviarTramaDinamica(t, r, 0);// s'allibera dins
        }else{
            status = enviarTramaDinamica(t, r, 1); // s'allibera dins
        }

        if(status == -1){//error , enviar trama ack KO , enviar trama dinamica si no pot enviar retorna -1 
            //, per tant hem de envair ack KO a origen
            printString(1, "Error forwarding. Cannot connect to realm.\n");
            char *data;
            asprintf(&data, "KO&%s", nom_origen);
            Trama *tramaACKKO = crearTrama(0x31, NULL, 0, NULL, strlen(data), data);
            free(data);
            int n_paraules = 0;
            char **ipPort = tokenizer(nom_origen, &n_paraules, ':');
            enviarTramaDirecte(tramaACKKO, atoi(ipPort[1]), ipPort[0],nom_desti);
            freeTokens(ipPort, n_paraules);
        }
        free(nom_origen);
        free(nom_desti);
    }
    st->thread_complete = THREAD_DONE;
    return NULL;
}

void closeServer(Regne *r){//per cntol +c
    if (r->fd_server > 0) {
     close(r->fd_server);
    }
}
