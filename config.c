

#include "config.h"




/*
    funcio per passar dun ip (int) a string
*/

char *IPToString (unsigned int ip){

    char *buffer;
    asprintf(&buffer, "%u.%u.%u.%u",
    (ip >> 24) & 0xFF, 
    (ip >> 16) & 0xFF, 
    (ip >> 8) & 0xFF, 
    (ip >> 0) & 0xFF);

    if (!strcmp(buffer, "250.250.250.250")){ 
        //aixo es el que hi ha quan es llegeix *.*.*.*
        free(buffer);
        asprintf(&buffer, "*.*.*.*");
    }

    return buffer;
}

/*
    funcio per passar de una string de ip a un int
*/

unsigned int strToIP (char *strIP, int strLen){

    unsigned int ip = 0; 
    int n_byte = 4;
    int i = 0;

    while (n_byte-- > 0){ 

        unsigned char byte = 0;
        while ((i < strLen) && (strIP[i] != '.')){

            byte = byte * 10;
            byte = byte + strIP[i] - '0';
            i++;
        }
        ip = ip << 8;
        ip = ip + byte;

        i++;
    }

    return ip;
}
/*
REALMS
*/
/*
    funcions d'alliberar memòria dinàmica
*/


void freeEnvoy(Envoy e){
    if (e.nomRegneViatge != NULL){
        free(e.nomRegneViatge);
    }
}

void freeRuta(Ruta r){
    if (r.nomDest != NULL){
        free(r.nomDest);
    }
    if (r.ipDest != NULL){
        free(r.ipDest);
    }
   
}
void freeRegne(Regne *r){

    if (r == NULL) return;

    if (r->nom != NULL){
        free(r->nom);
    }
    if (r->rutaArxius != NULL){
        free(r->rutaArxius);
    }

    if (r->ip != NULL){
        free(r->ip);
    }

    if (r->n_rutes != 0){
        for (int i = 0; i < r->n_rutes; i++){
            freeRuta(r->rutes[i]);
        }
        free(r->rutes);
    }
    

    if (r->n_envoys != 0){
        for (int i = 0; i < r->n_envoys; i++){
            freeEnvoy(r->envoys[i]);
        }
        free(r->envoys);
    }

    free(r);
}

/*
    funcions d'inicialitzar els strucs
*/

void initializeEnvoy (Envoy *e){

    e->status = 0;
    e->nomRegneViatge = NULL;
}

void initializeRuta (Ruta *r){

    r->nomDest = NULL;
    r->ipDest = NULL;
    r->portDest = 0;
    r->alianca = 'N'; 
    r->esDirectamentConectat = 0;
}

void initializeRegne (Regne *r){

    r->nom = NULL;
    r->rutaArxius = NULL;
    r->n_envoys = 0;
    r->ip = NULL;
    r->port = 0;
  
    r->n_rutes = 0;
    r->rutes = NULL;
}

/*
    funcions per copiar els structs
*/

void envoyCpy (Envoy *dest, Envoy *org){

    dest->status = org->status;

    if (dest->nomRegneViatge != NULL){
        free(dest->nomRegneViatge);
    }
    if (org->nomRegneViatge != NULL){
        dest->nomRegneViatge = malloc ((strlen(org->nomRegneViatge) + 1) * sizeof(char));
        strcpy(dest->nomRegneViatge, org->nomRegneViatge);
    }
}

void rutaCpy (Ruta *dest, Ruta *org){

    if (dest->nomDest != NULL){
        free(dest->nomDest);
    }
    if (org->nomDest != NULL){
        dest->nomDest = malloc ((strlen(org->nomDest) + 1) * sizeof(char));
        strcpy(dest->nomDest, org->nomDest);
    }

    if (dest->ipDest != NULL) free(dest->ipDest);
    if (org->ipDest != NULL) {
        dest->ipDest = malloc((strlen(org->ipDest) + 1) * sizeof(char));
        strcpy(dest->ipDest, org->ipDest);
    }

    /*if (dest->portDest != NULL) free(dest->portDest);
    if (org->portDest != NULL) {
        dest->portDest = malloc((strlen(org->portDest) + 1) * sizeof(int));
        strcpy(dest->portDest, org->portDest);
    }*/
    dest->portDest = org->portDest;

    dest->esDirectamentConectat = org->esDirectamentConectat;
    dest->alianca = org->alianca;
}

void regneCpy (Regne *dest, Regne* org){

    if (dest->nom != NULL){
        free(dest->nom);
    }
    if (org->nom != NULL){
        dest->nom = malloc ((strlen(org->nom) + 1) * sizeof(char));
        strcpy(dest->nom, org->nom);
    }

    if (dest->rutaArxius != NULL){
        free(dest->rutaArxius);
    }
    if (org->rutaArxius != NULL){
        dest->rutaArxius = malloc ((strlen(org->rutaArxius) + 1) * sizeof(char));
        strcpy(dest->rutaArxius, org->rutaArxius);
    }

    dest->n_envoys = org->n_envoys;
    if (dest->ip != NULL) free(dest->ip);
    if (org->ip != NULL) {
        dest->ip = malloc((strlen(org->ip) + 1) * sizeof(char));
        strcpy(dest->ip, org->ip);
    }
    dest->port = org->port;

    if (dest->n_rutes != 0){
        for (int i = 0; i < dest->n_rutes; i++){
            if (dest->rutes[i].nomDest != NULL){
                freeRuta(dest->rutes[i]);
            }
        }
        free(dest->rutes);

    }

    dest->n_rutes = org->n_rutes;

    if (org->n_rutes != 0){
        dest->rutes = malloc (org->n_rutes * sizeof(Ruta));

        for (int i = 0; i < org->n_rutes; i++){
            rutaCpy ( &(dest->rutes[i]), &(org->rutes[i]));
        }
    }
}

Regne *extractInfo (const char *nomFitxer) {

    int f = open(nomFitxer ,O_RDONLY);
    //printString(1, "nom del fitxer: ");
    //printString(1, (char *) nomFitxer);

    if (f == -1){
        printString(1, "Error opening configuration file.\n");
        return NULL;
    }
    //printString(1, "\nCreant regne...\n");
    Regne *r = malloc (sizeof(Regne));
    initializeRegne(r);

    int trash;
    r->nom = readUntil(f, '\n', &trash);
    if (r->nom != NULL) {
    size_t len;
    while ( (len = strlen(r->nom)) > 0 && (r->nom[len - 1] == '\n' || r->nom[len - 1] == '\r' || r->nom[len - 1] == ' ') ) 
    {
        r->nom[len - 1] = '\0';
    }
    }

    eliminarCaracter(r->nom, '&');
    //printString(1, "\nnom: ");
    //printString(1, r->nom);
    r->rutaArxius = readUntil(f, '\n', &trash);
    eliminarCaracter(r->rutaArxius, '&');
    //printString(1, "\nrutaArxius: ");
    //printString(1, r->rutaArxius);
    char *strEnvoys = readUntil(f, '\n', &trash);
    r->n_envoys = atoi(strEnvoys);
    free(strEnvoys);
    r->envoys = malloc (r->n_envoys * sizeof(Envoy));
    for (int i = 0; i < r->n_envoys; i++)
        initializeEnvoy(&(r->envoys[i]));
    //printString(1, "\nn envoys: ");
    //printUInt(1, r->n_envoys);
    //printChar(1, '\n');

    r->ip = readUntil(f, '\n', &trash); 
   size_t len;
    while ((len = strlen(r->ip)) > 0 && (r->ip[len - 1] == '\n' || r->ip[len - 1] == '\r' || r->ip[len - 1] == ' ')) {
    r->ip[len - 1] = '\0';
    }
    
    //printString(1, "\nip: ");
    //char *buffer = IPToString(r->ip);
    //printString(1, buffer);
    //printChar(1, '\n');
    //free(buffer);

    char *strPort = readUntil(f, ' ', &trash);
    r->port = (unsigned int) atoi(strPort);
    free(strPort);
    moveReadIdexUntil (f, '\n', &trash);



    while (1){

        Ruta ru;
        initializeRuta(&ru);

        ru.nomDest = readUntil(f, ' ', &trash);
        if (ru.nomDest == NULL) break;
       
        
      
        
        eliminarCaracter(ru.nomDest, '&');

        ru.ipDest = readUntil(f, ' ', &trash);
        ru.esDirectamentConectat = 1;
        if(!teCaracter(ru.ipDest,'*')){
            ru.esDirectamentConectat = 0;
        }
        char *strPortDest = readUntil(f, '\n', &trash);
        ru.portDest = (int) atoi(strPortDest);
        free(strPortDest);

       
        
        if (strcmp(ru.nomDest, "---") == 0) {
            //free(ru.nomDest);
            freeRuta(ru);
            continue;
        }
        r->rutes = realloc (r->rutes, (++(r->n_rutes)) * sizeof(Ruta));
        initializeRuta(&(r->rutes[r->n_rutes - 1]));
        rutaCpy ( &(r->rutes[r->n_rutes - 1]), &ru);
        freeRuta(ru);
    }
    close(f);
 
    return r;
}



/*
PRODUCTES
*/

void cpyProducte (Producte *dest, Producte *org){

    strcpy((*dest).name, (*org).name);
    (*dest).amount = (*org).amount;
    (*dest).weight = (*org).weight;

}
Producte *extrureProducte(int f) {
    Producte *p= malloc(sizeof(Producte));
    int size = read(f, p, sizeof(Producte));
    if(!size){
        free(p);
        return NULL;
    }
    return p;
}


Producte *extreureProductes(int fProductes, int *n_productes){
    *n_productes = 0;
    Producte * productes = NULL;
    while (1){
        Producte *p = extrureProducte (fProductes);
        if (p == NULL) break;
        productes = realloc (productes, ++(*n_productes) * sizeof(Producte));
        cpyProducte ( &(productes[(*n_productes) - 1]), p);
        free(p);
    }
    close(fProductes);
    return productes;
}

int comprovarRegneExisteix(Regne *r, char * nomRegneRuta) {
    
    char regneTrobat = 0;
    for (int i = 0; i < r->n_rutes; i++){
        //int nomRegLen = strlen(r->rutes[i].nomDest);
        if (!strcmp(nomRegneRuta,r->rutes[i].nomDest)){
            regneTrobat = 1;
        }
    }

    if (!regneTrobat){
        return 0; 
    }
    return 1; 
}











