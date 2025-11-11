
#include "producte.h"



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

void listProductsMaester(char *nomFitxer){

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
    listProducts(productes, n_productes);
    
    free(productes);
}

/*
START TRADE
*/
void sendCompra(char* nomRegneDest, char *nomMaester, Producte *productes, int numProductes, char *rutaArxius) {
    if (!nomRegneDest || !nomMaester || !productes || numProductes <= 0) {
        printString(1, "Cannot send trade: missing information\n");
        return;
    }
    char *missDir;
    asprintf(&missDir, ".%s", rutaArxius); 
    pid_t pid = fork();
    if (pid == 0) {
        execlp("mkdir", "mkdir", "-p", missDir, (char*)NULL);
        _exit(1);
    } else if (pid > 0) {
        waitpid(pid, NULL, 0);
    } else {
        printString(1, "Fork error\n");
        free(missDir);
        return;
    }
    free(missDir);
    char *nomFitxer;
    asprintf(&nomFitxer, ".%s/Trade_%s-_%s-.txt", rutaArxius, nomMaester, nomRegneDest);
    int fd = open(nomFitxer, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        printString(1, "Error opening trade file\n");
        free(nomFitxer);
        return;
    }
    char *miss2; 
    for (int i = 0; i <numProductes; i++) { 
        asprintf(&miss2, "%s:%d\n", productes[i].name,productes[i].amount); 
        write(fd, miss2, strlen(miss2)); 
        free(miss2); 
    } 
    char *miss; 
    asprintf(&miss,"Trade list sent to %s\n",nomRegneDest); printString(1,miss); free(miss);

    close(fd);
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

void startTrade(Regne * regneActual, char* comand){
    
    int areTrading = 1;
   
    int numProductes = 0;
    int comandaTradeN = -1;
    char nomProducte[100];
    int quantitat= 0;

     int n_paraules = 0;
    char **paraules = tokenizer(comand, &n_paraules);
    Producte *productes = NULL;
    char *miss;
    Producte *productesDisponibles = NULL;
    int n_productesDisponibles = 5;
    

    char *nomRegneDest = strdup(paraules[2]);
    asprintf(&miss, "Entering trade mode with %s\n", nomRegneDest);
    printString(1, miss);
    free(miss);
    productesDisponibles = crearProductesProvisionals(n_productesDisponibles);//provisional
    mostrarProductesCompra(productesDisponibles, n_productesDisponibles);
    freeTokens(paraules, n_paraules);

    do{
        printString(1,"Trade> ");
        comandaTradeN=-1;
        char comandaTrade[100];
        read(0, comandaTrade, 100);
        removeFirstEnter(&comandaTrade, 100);

        char **paraulesTrade = tokenizer(comandaTrade, &n_paraules);
        
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
            continue;
        }

        switch (comandaTradeN){
            case 0: //add
                if (n_paraules != 3) {
                    printString(1, "add <product_name> <quantity>\n");
                    break;
                }
                strcpy(nomProducte, paraulesTrade[1]);
                if (!comprovarSiNumero(paraulesTrade[2])) {
                    printString(1, "You have to put a positive number.\n");
                    break;
                }
                quantitat = atoi(paraulesTrade[2]);
                if (!comprovarExistexProducte(nomProducte, productesDisponibles, n_productesDisponibles)) {
                    printString(1, "Product not found\n");
                    break;
                }
                addProducteTrade(&productes, &numProductes, nomProducte, quantitat);
                break;

            case 1: //remove
                
                if (n_paraules != 3) {
                    printString(1, "remove <product_name> <quantity>\n");
                    break;
                }
                strcpy(nomProducte, paraulesTrade[1]);
                quantitat = atoi(paraulesTrade[2]);
                removeProducteTrade(&productes, &numProductes, nomProducte, quantitat);
                break;

            case 2: //send
                sendCompra(nomRegneDest, regneActual->nom, productes, numProductes, regneActual->rutaArxius);
                areTrading = 0;
                break;

            case 3: //cancel
                cancelCompra(nomRegneDest);
                areTrading = 0;
                break;
        }


        freeTokens(paraulesTrade, n_paraules);

    } while(areTrading);
    free(nomRegneDest);
    free(productes);
    free(productesDisponibles);
    productes = NULL;
}



//no borrar , no cal per fase 1
/*void actualitzarProductes(char *nomFitxer, Producte *productes, int n_productes){
    int f = open (nomFitxer, O_WRONLY | O_TRUNC | O_CREAT, 0644);
    if (f == -1){
        printString(1, "Error opening product file\n");
        return;
    }
    for (int i = 0; i < n_productes; i++){
        write (f, &(productes[i]), sizeof(Producte));
    }
    close(f);
}*/
