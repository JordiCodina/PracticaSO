
#include "producte.h"



/*

@brief Funció que rep els productes enviats per un aliat i els desa en un fitxer.
@param r Punter a la estructura Regne que conté la informació del regne actual.
@param socket Descriptor del socket utilitzat per la comunicació.
@param t Punter a la estructura Trama que conté la informació de la trama rebuda.
@param indexAliat Índex del regne aliat a la taula de rutes.

*/

void rebreProductes(Regne *r, int socket, Trama *t,int indexAliat) {

    // DATA = <FileName>&<FileSize>&<MD5SUM>
    int n_paraules = 0;
    char **paraules = tokenizer(t->data, &n_paraules, '&');
    if (n_paraules != 3) {
        printString(1, "Error: trama 0x12 mal formada.\n");
        freeTokens(paraules, n_paraules);
        return;
    }

    char *fileName = paraules[0];
    int fileSize   = atoi(paraules[1]);
    char *MD5Esperat = paraules[2];

    Trama *ack = crearTrama(0x31, NULL, 0, NULL, 2, "OK");
    write(socket, ack, sizeof(Trama));
    free(ack);

    char *missDir;
    asprintf(&missDir, ".%s", r->rutaArxius);
    char *fitxerPath;
    asprintf(&fitxerPath, "%s/%s", missDir, fileName);
    int fd = open(fitxerPath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
 
    if (fd < 0) {
        printString(1, "Error obrint fitxer d'inventari.\n");
        free(fitxerPath);
        free(missDir);
        freeTokens(paraules, n_paraules);
        return;
    }
    lseek(fd, 0, SEEK_END);
    int bytesRebuts = 0;
    //comencem a rebre les trames amb les dades del fitxer productes
    while (bytesRebuts < fileSize) {
        Trama tramaRebuda;
        int n = read(socket, &tramaRebuda, sizeof(Trama));
        if (n <= 0) {
            printString(1, "Connection closed before inventory was complete.\n");
            close(fd);
            free(fitxerPath);
            free(missDir);
            freeTokens(paraules, n_paraules);
            return;
        }

        if (tramaRebuda.tipus != 0x13) {
            printString(1, "Trama is incorrect\n");
            close(fd);
            free(fitxerPath);
            free(missDir);
            freeTokens(paraules, n_paraules);
            return;
        }
        //escrivim les dades rebudes al fitxer temp de la carpeta del regne 
        write(fd, tramaRebuda.data, tramaRebuda.dataLength);
        bytesRebuts += tramaRebuda.dataLength;
    }
    close(fd);
    char *md5 = calcularMD5SUM(fitxerPath);
    if (!md5) {
        printString(1, "Error calculant MD5.\n");
        free(fitxerPath);
        free(missDir);
        freeTokens(paraules, n_paraules);
        return;
    }

    Trama *ackMD5;
    if (strcmp(md5, MD5Esperat) == 0) {
        ackMD5 = crearTrama(0x32, NULL, 0, NULL, strlen("CHECK_OK"), "CHECK_OK");
    } else {
        ackMD5 = crearTrama(0x32, NULL, 0, NULL, strlen("CHECK_KO"), "CHECK_KO");
    }
    enviarTramaDirecte(ackMD5,r->rutes[indexAliat].portDest,r->rutes[indexAliat].ipDest,r->nom);
        listProductsMaester(fitxerPath,1,r->rutes[indexAliat].nomDest);

    free(md5);
    free(fitxerPath);
    free(missDir);
    freeTokens(paraules, n_paraules);
}


void listProductsAliat(Regne *r, char *nomAliat/*,int* exitFlag*/) {
        
    if (!comprovarRegneExisteix(r, nomAliat)){
           printString(1, "That Realm cannot be found , remember its case sensitive.\n");
           return;
    }
    int esAliat=0;
    int indexAliat=-1;
    for(int i=0;i<r->n_rutes;i++){
        if(strcmp(r->rutes[i].nomDest,nomAliat)==0){
            if(r->rutes[i].alianca=='S'){
                esAliat=1;
                indexAliat=i;
                char *buffer;
                asprintf(&buffer, "Direct route available to %s (allied).No hops required.\n", nomAliat);
                printString(1, buffer);
                free(buffer);
                break;
            }else if(r->rutes[i].alianca=='I'){
                char *buffer;
                asprintf(&buffer, "ERROR: Realm %s is inactive. Cannot receive products now.\n", nomAliat);
                printString(1, buffer);
                free(buffer);
                return ;
            }else{
                char *buffer;
                asprintf(&buffer, "ERROR: You must have an alliance with %s to trade\n", nomAliat);
                printString(1, buffer);
                free(buffer);
                return;
            }
            break;
        }
    }
    if(!esAliat){
        return;
    }
    Trama *t = crearTrama(0x11, r->ip, r->port, nomAliat, 0, r->nom);
    int socket=enviarTramaDirecteSenseTancar(t, r->rutes[indexAliat].portDest, r->rutes[indexAliat].ipDest, nomAliat);
    if(socket<0){
        printString(1, "Error sending trama to ally. Probably is disconnected.\n");
        return;
    }
    printString(1, "Waiting for product list from ally...\n");

    Trama *tramaPeticioLlista = malloc(sizeof(Trama));
    int n=read(socket, tramaPeticioLlista, sizeof(Trama));
    if(n<=0){
        free(tramaPeticioLlista);
        printString(1, "Did not receive product list from ally.\n");
        close(socket);
        return;
    }
    
    if(tramaPeticioLlista!=NULL || tramaPeticioLlista->tipus==0x69){
       //si no hem llegit res o es un nack , sera la 0x12 que es la iniciat per rebre productes
        if (tramaPeticioLlista && tramaPeticioLlista->tipus == 0x12) {
            rebreProductes(r, socket, tramaPeticioLlista, indexAliat);
        }
    }else{
        printString(1, "Did not receive product list from ally.\n");
    }
    free(tramaPeticioLlista);
    close(socket);
}

/*
LIST PRODUCTS
*/ 

void listProducts(Producte *p, int n) {
    char *miss;

    printString(1,"--- Trade Ledger ---\n");
    printString(1,"Item                         | Value (Gold) | Weight (Stone)\n");
    printString(1,"--------------------------------------------------------------\n");
    for (int i = 0; i < n; i++) {
        asprintf(&miss, "%-28s | %-12d | %-14.1f\n",p[i].name, p[i].amount, p[i].weight);
        printString(1, miss);
        free(miss);
    }
    printString(1,"--------------------------------------------------------------\n");
    asprintf(&miss,"Total Entries: %d\n\n", n);
    printString(1, miss);
    free(miss);
}
void listProductsMaesterAliat(Producte *p, int n,char *nomAliat) {
    
    printString(1, "Products received from ");
    printString(1, nomAliat);
    printString(1, ":\n");
    char *miss2;
    for (int i = 0; i < n; i++) {
        asprintf(&miss2, "%d. %s (%d units)\n", i + 1, p[i].name, p[i].amount);
        printString(1, miss2);
        free(miss2);
    }
}


void listProductsMaester(char *nomFitxer, int opcio,char * nomAliat) {

    int n_productes = 0;
    int fd = open(nomFitxer, O_RDONLY);
    if(fd == -1) {
        char *miss;
        asprintf(&miss,"Product file %s not found\n", nomFitxer);
        printString(1, miss);
        free(miss);
        return;
    }

    Producte *productes = extreureProductes(fd,&n_productes);
    close(fd);
    if(productes == NULL){
        printString(1, "No products found\n");
     return;
    }
    if(n_productes == 0){ 
        printString(1, "No products found\n");
        free(productes);
        return;
    }
    if(opcio==1){
        listProductsMaesterAliat(productes, n_productes, nomAliat);

    }else{
    listProducts(productes, n_productes);
    }
    
    free(productes);
}

/*
START TRADE
*/

char* crearRutaComanda(char * rutaArxius, char * nomMaester, char * nomRegneDest){
        char *nomFitxer;
        asprintf(&nomFitxer, ".%s/%s-_%s-_commands.txt", rutaArxius, nomMaester, nomRegneDest);
        return nomFitxer;
    }
int crearFitxerComandes(char *nomFitxer){
        
        int fd = open(nomFitxer, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd == -1) {
            printString(1, "Error opening trade file\n");
            return -1;
        }
        return fd;
    }

/*
    
@brief Funció que envia una comanda de compra a un regne aliat.Espera el seur tetorn de resposta , incolent el md5sum.Si tot va be actualitza l'stock .
@param nomRegneDest Nom del regne aliat al qual s'envia la comanda.
@param nomMaester Nom del maester que envia la comanda.
@param productes Array de productes que es volen comprar.
@param numProductes Nombre de productes a comprar.
@param rutaArxius Ruta dels arxius del regne actual.
@param r Punter a la estructura Regne que conté la informació del regne actual.

    
*/
void sendCompra(char* nomRegneDest, char *nomMaester, Producte *productes, int numProductes, char *rutaArxius, Regne *r) {
    if (!nomRegneDest || !nomMaester || !productes || numProductes <= 0) {
        printString(1, "Cannot send trade: missing information\n");
        return;
    }
    //comprovar que el regne existeix i es aliat
    int indexRuta=-1;
    for (int i=0;i<r->n_rutes;i++){
        if(strcmp(r->rutes[i].nomDest,nomRegneDest)==0){            
            indexRuta=i;
            break;
        }
    }
    if(r->rutes[indexRuta].alianca!='S'){    
        if(r->rutes[indexRuta].alianca=='I'){
            printString(1, "Allied is inactive.Cannot send trade.\n");    
            return;
        }
    }
    //per si de cas creem directori i el fitxer de comandes i els posem a dins
    crearDirectori(r->rutaArxius);
    char *nomFitxer;
    nomFitxer=crearRutaComanda(rutaArxius, nomMaester,nomRegneDest);//aqui nomes crees la string de ruta
    int fd=crearFitxerComandes(nomFitxer);//aqui crees el fitxer reiniciantlo 
    if(fd==-1){
        free(nomFitxer);
        return;
    }
    char *miss2; 
    for (int i = 0; i <numProductes; i++) { 
        asprintf(&miss2, "%s:%d\n", productes[i].name,productes[i].amount); //posem els prodcutes al fitxer de comandes
       
        write(fd, miss2, strlen(miss2)); 
        free(miss2); 
    } 
    close(fd);
    //abans hem fet un write i ara toca enviar la comanda per tant abans l'hem tancat i ara el tornem a obrir per llegir
    int fdList=open(nomFitxer, O_RDONLY);
    if(fdList<0){
        printString(1, "Error opening trade file to read\n");
        free(nomFitxer);
        return;
    }
    //preparar dades per enviar la trama 0x14
    int size=getSize(fdList);
    char *md5 = calcularMD5SUM(nomFitxer);
    if (!md5) {
        printString(1, "Error calculating MD5 for trade file\n");
        close(fdList);
        free(nomFitxer);
        return;
    }
    char data[275];
    snprintf(data, sizeof(data), "%s&%d&%s", nomFitxer, size, md5);
    free(md5);
    //enviar trama 0x14
    Trama *tramaPeticioComanda=crearTrama(0x14,r->ip , r->port, nomRegneDest, strlen(data), data);
    int socket=enviarTramaDirecteSenseTancar(tramaPeticioComanda, r->rutes[indexRuta].portDest, r->rutes[indexRuta].ipDest, nomRegneDest);//el free de trama es fa aqui dintre
    if(socket<0){
        printString(1, "Error sending trade request to ally. Probably is disconnected.\n");
        close(fdList);
        free(nomFitxer);
        return;
    }
    //eseperar ack/nack de la comanda
    Trama * ackPeticioComanda=crearTrama(0, NULL, 0, NULL, 0, NULL);
    int n=read(socket, ackPeticioComanda, sizeof(Trama));
    if(n<=0){
        free(ackPeticioComanda);
        printString(1, "Did not receive ACK/NACK for trade request.\n");
        close(fdList);
        free(nomFitxer);
        close(socket);
        return;
    }
    if (ackPeticioComanda->tipus != 0x31) {  
        printString(1, "Trade request denied by ally.\n");
        close(fdList);
        free(nomFitxer);
        free(ackPeticioComanda);
        return;
    }
    free(ackPeticioComanda);

    //si tot va be aqui enviem la llista de productes en trames 0x15
        char *listData;
        printString(1, "Sending list ...\n"); 
        int  startPos = 0;
        Trama *trama=NULL;
        while(startPos< size){
            usleep(5000);
            if (r->exit && *(r->exit)) {
                close(fdList);
                close(socket);
                free(nomFitxer);
                return;
            }
            if(startPos+275>size){
                 
                listData=readFd(fdList,startPos,size-startPos);
                trama = crearTrama(0x15, r->ip, r->port, r->rutes[indexRuta].nomDest , size-startPos,listData);

            }else{
                listData=readFd(fdList,startPos,275);
                trama = crearTrama(0x15, r->ip, r->port, r->rutes[indexRuta].nomDest , 275,listData);
            }
            enviarTramaSocket(trama,socket);
            free(trama);
            startPos+=275;

            free(listData);
            
        }
        close(fdList);
        //esperar ack/nack amb md5
            Trama *tramaPeticioComandaAckmd5=crearTrama(0, NULL, 0, NULL, 0, NULL);
        int n4=read(socket, tramaPeticioComandaAckmd5, sizeof(Trama));
        if(n4<=0){
            free(tramaPeticioComandaAckmd5);
            printString(1, "Did not receive ACK/NACK for trade list.\n");
            free(nomFitxer);
            close(socket);
            return;
        }
        if (tramaPeticioComandaAckmd5->tipus != 0x32 || strcmp(tramaPeticioComandaAckmd5->data,"CHECK_OK")!=0) {  
            printString(1, "MD5 checksum does not match for trade list.\n");
            free(tramaPeticioComandaAckmd5);
            free(nomFitxer);
            close(socket);
            return;
        }
        free(tramaPeticioComandaAckmd5);        
    char *miss; 
    asprintf(&miss,"Trade list sent to %s\n",nomRegneDest); printString(1,miss); free(miss);
    //LLEGEIR TRAMA 0X16
    Trama *tramaConfirmacioComanda=crearTrama(0, NULL, 0, NULL, 0, NULL);
    int n2=read(socket, tramaConfirmacioComanda, sizeof(Trama));
    if(n2<=0){
        close(fdList);
        free(nomFitxer);
        free(tramaConfirmacioComanda);
        close(socket);
        return;
    }
    //aqui en funcio de lo que ha retornat la trama apliquem canvis a l'stock o no
    if(tramaConfirmacioComanda->tipus==0x16){
        //aqui actualitzem el stock si la comanda ha anat be
        if(strcmp(tramaConfirmacioComanda->data,"OK")==0){
            char *nomStock;
            asprintf(&nomStock, "stock%s.db", r->nom);
            int n_stock = 0;
            int fdProductes = open(nomStock, O_RDONLY);
            if (fdProductes < 0) {
                printString(1, "Error opening stock file to read\n");
                free(nomStock);
                free(tramaConfirmacioComanda);
                close(socket);
                return;
            }
            Producte *stock = extreureProductes(fdProductes, &n_stock);
            stock = realloc(stock, (n_stock + numProductes) * sizeof(Producte));
            close(fdProductes);
            for (int i = 0; i < numProductes; i++) {
                afegirAlStock(stock, &n_stock,productes[i].name,productes[i].amount,0);
            }
            actualitzarProductes(nomStock, stock, n_stock);
            free(stock);
            free(nomStock);
            printString(1, ">>> Order accepted by TheVale. Stock updated.\n");
        }
        else if(strcmp(tramaConfirmacioComanda->data,"REJECT&OUT_OF_STOCK")==0){
            printString(1, ">>> Order rejected. One or more products are out of stock.\n");
        }
        else if(strcmp(tramaConfirmacioComanda->data,"REJECT&UNKNOWN_PRODUCT")==0){
            printString(1, ">>> Order rejected. Unknown product in the list.\n");
        }else{
            printString(1, ">>> Order rejected. Unknown error.\n");
        }

    }
    free(tramaConfirmacioComanda);
    close(fd);
    close(socket);
    free(nomFitxer);
}


void mostrarProductesCompra(Producte *productesDisponibles, int n_productes){
    printString(1, "Available products: ");
    for (int i = 0; i < n_productes; i++){

        printString(1,productesDisponibles[i].name);
        printString(1," ");

    }
    printString(1,"\n");

}
void cancelCompra(char* nomRegneDest){
    char *miss;
    asprintf(&miss,"Cancelling trade mode with %s\n", nomRegneDest);
    printString(1, miss);
    free(miss);
}

Producte * crearProductesProvisionals(int n) {
    Producte *productes = malloc (n * sizeof(Producte));
    for (int i = 0; i < n; i++){
        sprintf(productes[i].name, "prod%d", i + 1);
        productes[i].amount = 5 +n;
        productes[i].weight = 20 +n;
    }
    return productes;
}
int comprovarExistexProducte(char *nomProducte, Producte *productesDisponibles, int n_productes){
    for (int i = 0; i < n_productes; i++){
        if (!strcmp(nomProducte, productesDisponibles[i].name)){
            return 1;
        }
    }
    return 0;
}

void addProducteTrade(Producte **productes, int *numProductes, char *nomProducte, int quantitat) {
    if (quantitat <= 0) {
        printString(1, "You have to put a positive number.\n");
        return;
    }
    for (int i = 0; i < *numProductes; i++) {//per si el producte existeix
        if (strcmp((*productes)[i].name, nomProducte) == 0) {
            (*productes)[i].amount += quantitat;
            return;
        }
    }

    (*numProductes)++;
    *productes = realloc(*productes, sizeof(Producte) * (*numProductes));
    if (*productes == NULL) {
        printString(1, "Error reallocating memory\n");
        return;
    }

    strcpy((*productes)[*numProductes - 1].name, nomProducte);
    (*productes)[*numProductes - 1].weight = 0;
    (*productes)[*numProductes - 1].amount = quantitat;
}

void removeProducteTrade(Producte **productes, int *numProductes, char *nomProducte, int quantitat) {
    if (*productes == NULL || *numProductes == 0) {
        printString(1, "You didn't add any products\n");
        return;
    }

    if (quantitat <= 0) {
        printString(1, "Amount invalid\n");
        return;
    }

    int trobat = 0;
    for (int i = 0; i < *numProductes; i++) {
        if (strcmp((*productes)[i].name, nomProducte) == 0) {
            trobat = 1;
            if ((*productes)[i].amount <= quantitat) {
                for (int j = i; j < (*numProductes) - 1; j++) {
                    (*productes)[j] = (*productes)[j + 1];
                }
                (*numProductes)--;
                *productes = realloc(*productes, sizeof(Producte) * (*numProductes));
                if (*productes == NULL && *numProductes > 0) {
                    printString(1, "Error realloc memory\n");
                    return;
                }
            } else {
                (*productes)[i].amount -= quantitat;
            }
            break;
        }
    }
    if (!trobat) {
        printString(1, "Product not found in trade list\n");
    }
}


void startTrade(Regne *regneActual, char *comand, fd_set *master_set, int *max_fd) {
    
    
    int areTrading = 1;
   
    int numProductes = 0;
    int comandaTradeN = -1;
    char nomProducte[100];
    int quantitat= 0;

     int n_paraules = 0;
    char **paraules = tokenizer(comand, &n_paraules,' ');
   //comprovar que el regne existeix i es aliat
    int esAliat=0;
    for(int i=0 ;i<regneActual->n_rutes;i++){
        if(strcmp(regneActual->rutes[i].nomDest,paraules[2])==0){
            if(regneActual->rutes[i].alianca=='S'){
                esAliat=1;
                break;
            }
        }
    }
    //si no es alita fora
    if(!esAliat){
        printString(1, "You are not allied with this Realm or its inactive.\n");
        freeTokens(paraules, n_paraules);
        return;
    }
    Producte *productes = NULL;
    Producte *productesDisponibles = NULL;
    int n_productesDisponibles = 0;
    char *nomRegneDest = strdup(paraules[2]);

    char *miss;
    asprintf(&miss, "Entering trade mode with %s\n", nomRegneDest);
    printString(1, miss);
    free(miss);
    //obrim el fitxer de productes disponibles que tenim guardat a la nostre carpeta depres de fer LIST PRODUCTS
    char*missDir;
    asprintf(&missDir, "./%s/stock%s.db",regneActual->nom,nomRegneDest);
    int fdFitxerProductes = open(missDir, O_RDONLY);
    if (fdFitxerProductes == -1) {
        printString(1, "No products available . Use LIST PRODUCTS first.\n");
        free(nomRegneDest);
        freeTokens(paraules, n_paraules);
        free(missDir);  
        return;
    }
    free(missDir);
    //extreiem productes disponibles del fitxer de productes de la nostre carpeta
    productesDisponibles = extreureProductes(fdFitxerProductes, &n_productesDisponibles);
    //printf
    listProductsMaesterAliat(productesDisponibles, n_productesDisponibles, nomRegneDest);
    if (productesDisponibles == NULL || n_productesDisponibles == 0) {
        printString(1, "No products available . Use LIST PRODUCTS first.\n");
        close(fdFitxerProductes);
        
        free(nomRegneDest);
        freeTokens(paraules, n_paraules);
        return;
    } 
    freeTokens(paraules, n_paraules);
    printString(1,"Trade> ");
    //aqui comneça la terminal de trade(igual que terminal.c)
    do{
        comandaTradeN=-1;
        n_paraules = 0;
        if(*(regneActual->exit) == 1){
            comandaTradeN = 7;
            break;
        }
        char *comandaTrade = escoltarTot(master_set, max_fd, regneActual);
        if (comandaTrade == NULL) {
            continue;
        }
        
        char **paraulesTrade = tokenizer(comandaTrade, &n_paraules,' ');
        if(n_paraules==0){
            freeTokens(paraulesTrade, n_paraules);
            free(comandaTrade);
            printString(1,"Trade> ");
            continue;
        }
        if (strcasecmp (paraulesTrade[0], "add") == 0){
            comandaTradeN = 0;
        }
        else if (strcasecmp (paraulesTrade[0], "remove") == 0){
            comandaTradeN = 1;
        }
        else if (strcasecmp (paraulesTrade[0], "send") == 0){
            comandaTradeN = 2;
        }
        else if (strcasecmp (paraulesTrade[0], "cancel") == 0){
            comandaTradeN = 3;
        } else {
            printString(1, "Invalid command\n");
            freeTokens(paraulesTrade, n_paraules);
            printString(1,"Trade> ");
            continue;
        }

        switch (comandaTradeN){
            case 0: //add
                if (n_paraules < 3) {
                    printString(1, "add <product_name> <quantity>\n");
                    
                    break;
                }
                nomProducte[0] = '\0';
                for (int i = 1; i < n_paraules - 1; i++) {
                    strcat(nomProducte, paraulesTrade[i]);
                    if (i < n_paraules - 2) strcat(nomProducte, " ");
                }

                if (!comprovarSiNumero(paraulesTrade[n_paraules - 1])) {
                    printString(1, "You have to put a positive number.\n");
                    break;
                }
                quantitat = atoi(paraulesTrade[n_paraules - 1]);

                if (!comprovarExistexProducte(nomProducte, productesDisponibles, n_productesDisponibles)) {
                    printString(1, "Product not found\n");
                    break;
                }
                addProducteTrade(&productes, &numProductes, nomProducte, quantitat);
                break;

            case 1: //remove
                
                 if (n_paraules < 3) {
                    printString(1, "remove <product_name> <quantity>\n");
                    break;
                }
                nomProducte[0] = '\0';
                for (int i = 1; i < n_paraules - 1; i++) {
                    strcat(nomProducte, paraulesTrade[i]);
                    if (i < n_paraules - 2) strcat(nomProducte, " ");
                }
                quantitat = atoi(paraulesTrade[n_paraules - 1]);
                removeProducteTrade(&productes, &numProductes, nomProducte, quantitat);
                break;

            case 2: //send
                sendCompra(nomRegneDest, regneActual->nom, productes, numProductes, regneActual->rutaArxius, regneActual);
                areTrading = 0;
                break;

            case 3: //cancel
                cancelCompra(nomRegneDest);
                areTrading = 0;
                break;
        }


        freeTokens(paraulesTrade, n_paraules);
        free(comandaTrade);
        if(areTrading){
        printString(1,"Trade> ");
        }
    } while(areTrading);
    free(nomRegneDest);
    free(productes);
    free(productesDisponibles);
    productes = NULL;
}



int treureDelStock(Producte *productes, int *n_productes, const char *nom, int quantitat) {
    for (int i = 0; i < *n_productes; i++) {
        if (strcmp(productes[i].name, nom) == 0) {
            if (productes[i].amount < quantitat)
                return 0;
            productes[i].amount -= quantitat;
            if (productes[i].amount <= 0) {
                for (int j = i; j < *n_productes - 1; j++)
                    productes[j] = productes[j + 1];
                (*n_productes)--;
            }
            return 1; 
        }
    }
    return -1; 
}


void afegirAlStock(Producte *productes, int *n_productes,const char *nom, int quantitat, float pes){
    for (int i = 0; i < *n_productes; i++) {

        if (strcmp(productes[i].name, nom) == 0) {
            productes[i].amount += quantitat;
            return; // ja està
        }
    }
    strncpy(productes[*n_productes].name, nom, sizeof(productes[*n_productes].name));
    productes[*n_productes].name[sizeof(productes[*n_productes].name) - 1] = '\0';

    productes[*n_productes].amount = quantitat;
    productes[*n_productes].weight = pes;
    (*n_productes)++;
}


void actualitzarProductes(char *nomFitxer, Producte *productes, int n_productes){
    int f = open (nomFitxer, O_WRONLY | O_TRUNC | O_CREAT, 0644);
    if (f == -1){
        printString(1, "Error opening product file\n");
        return;
    }
    for (int i = 0; i < n_productes; i++){  
        
        write (f, &(productes[i]), sizeof(Producte));
    }
    close(f);
}
