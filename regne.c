#include "regne.h"



int comprovarRutaRegneValid(char *str,int n_rutes , Ruta *rutes){
    int esValid=0;
    for(int i=0; i<n_rutes;i++){
        if(strcmp(str,rutes[i].nomDest)==0 && strcmp(str,"DEFAULT")!=0){
            esValid=1;
            break;
        }
    }
    return esValid;
}


/*
    LIST REALMS
*/

void listRealms (Regne *r){

    for (int i = 0; i < r->n_rutes; i++){

        if(strcmp(r->rutes[i].nomDest,"DEFAULT")==0) continue;//no cal mostrar el default
        char *buffer;
        asprintf(&buffer, "- %s\n", r->rutes[i].nomDest);
        printString(1,buffer);
        free(buffer);
    }
}


/*
    PLEDGE
*/

void sendEnvoy(Regne *r, char *casa){//no per aquesta fase

    char envoy_sent = 0;

    for (int i = 0; i < r->n_envoys; i++){
        if (r->envoys[i].status == 0){ // 0 -> idle
            r->envoys[i].status = 1;
            r->envoys[i].nomRegneViatge = malloc (sizeof(char) * (strlen(casa) + 1));
            strcpy(r->envoys[i].nomRegneViatge, casa);

            char *buffer;
            asprintf(&buffer, "Pledge sent to %s. Envoy %d is on its way.\n", casa, i + 1);
            printString(1,buffer);
            free(buffer);

            envoy_sent = 1;
            break;
        }

    }

    if (!envoy_sent){
        printString(1, "All envoys are on a mission.\n");
    }
}


extern int exitFlag;

void *runTimeoutRuta(void *arg) {
    Ruta *r = (Ruta *)arg;
    const double timeout = 120.0; 
    time_t inici = time(NULL);

    while (1) {
        sleep(1);
        time_t ara = time(NULL);
        double diff = difftime(ara, inici);
        if (diff >= timeout) {
            if (r->alianca == 'P')
                r->alianca = 'T';
            break;
        }
        if(exitFlag){
            printf("Exiting timeout thread due to exitFlag\n");
            break;
        }
        if (r->alianca == 'S' || r->alianca == 'N') {
            break;
        }
    }

    return NULL;
}
void pledge (Regne *r,char **paraules){

    if (!comprovarRegneExisteix(r, paraules[1])){
           printString(1, "That Realm cannot be found , remember its case sensitive.\n");
           return;
    }
    char * filename=malloc(FILENAME_SIGIL_SIZE*sizeof(char));
    snprintf(filename, FILENAME_SIGIL_SIZE, "segells/%s",paraules[2]);
    int fd=open(filename, O_RDONLY);
    if(fd==-1){
        printString(1, "Error opening  file\n");
        free(filename);
        
        return;
    }
    int indexRuta=-1;
    for(int i=0;i<r->n_rutes;i++){
        
        if(strcmp(r->rutes[i].nomDest,paraules[1])==0){
            if( r->rutes[i].alianca=='S'){
                printString(1, "You are already allied with this Realm.\n");
                return;
            }
            if( r->rutes[i].alianca=='P'){
                printString(1, "You have already sent an alliance request to this Realm.\n");

                return;
            }
            r->rutes[i].alianca='P';
            indexRuta=i;
            //printString(1, "Alliance request sent.\n");
            break;
        }
    }
    char data[DATA_SIZE];
       
    int size=getSize(fd);
    snprintf(data, DATA_SIZE, "%s&%s&%d&%s",r->nom,paraules[2],size,"MDSUM5");
     
    Trama *trama = crearTrama( 0x01, r->ip, r->port, paraules[1], strlen(data), data);
   
    enviarTramaDinamica(trama, r, 0);//trama s'allibera aqui
    
    close(fd);
    free(filename); 
    
    char *buffer;
        asprintf(&buffer, "Pledge sent to %s. Envoy 1 is on its way.\n", paraules[1]);//tambe s'ha de posar quin envoy
        printString(1, buffer);
        free(buffer);

    pthread_t th;
    pthread_create(&th, NULL, runTimeoutRuta, (void *)&r->rutes[indexRuta]);
    pthread_join(th, NULL);
    /*time_t inici = time(NULL);         
    const double timeout = 120.0;
    while(1){
        sleep(1);
        time_t ara = time(NULL);
        double diff = difftime(ara, inici);
        if (diff >= timeout) {                      
           // printf("Timeout de 2 minuts.\n");
            r->rutes[indexRuta].alianca='T';//T de timeout
            break;
        }
        if(r->rutes[indexRuta].alianca=='S' || r->rutes[indexRuta].alianca=='N'){
            break;
        }
    }*/
} 



                
/*
    PLEDGE RESPOND
*/


void pledgeRespond ( Regne *r,char** paraules,int n_paraules,int * exitf){

    if(paraules==NULL || n_paraules<4){
        printString(1, "Please specify the Realm and response (accept/reject).\n");
        return;
    }
    int trobat=-1;
    for(int i=0;i<r->n_rutes;i++){
        if(strcmp(r->rutes[i].nomDest,paraules[2])==0){
            trobat=i;
            break;
        }
    }
    if(trobat<0){
        printString(1, "That Realm cannot be found.\n");
        return;
    }
    if(r->rutes[trobat].alianca=='S'){
        printString(1, "You are already allied with this Realm.\n");
        return;
    }
    if(r->rutes[trobat].alianca=='N'){
        printString(1, "No alliance request.\n");
        return;
    }

    if(r->rutes[trobat].alianca=='P'){
     if(strcasecmp(paraules[3], "reject")==0){
        //respondPledge(0);
        enviarTramaDirecte(crearTrama(0x03, r->ip, r->port, r->nom, strlen("REJECT"), "REJECT"), r->rutes[trobat].portDest, r->rutes[trobat].ipDest, paraules[2]);
        r->rutes[trobat].alianca='N';
        return;
    }
     if(strcasecmp(paraules[3], "accept")==0){
       // printf(">>> Alliance with %s forged successfully\n", paraules[2]);
        //printf("portDest: %d, ipDest: %s, nomDest: %s\n", r->rutes[trobat].portDest, r->rutes[trobat].ipDest, paraules[2]);
        //printf("ruta: %s %d\n", r->rutes[trobat].ipDest, r->rutes[trobat].portDest);
        int socket=enviarTramaDirecteSenseTancar(crearTrama(0x03, r->ip, r->port, r->nom, strlen("ACCEPT"), "ACCEPT"), r->rutes[trobat].portDest, r->rutes[trobat].ipDest, paraules[2]);
        if(socket<0){
            return;
        }
        Trama * t = waitForRead(3.0,socket,exitf);
       // printf("adreçatl: %p<-\n", &tl);
       
       
        if(t!=NULL){
        //si es ack o nack
        //printf("Ack received from %s: %s\n", paraules[2], t->data);
        if(t->tipus==0x31){//ack
            //printf(">>> Alliance with %s forged successfully\n", paraules[2]);
            char *miss;
            asprintf(&miss, "Alliance with %s established.\n", paraules[2]);
            printString(1, miss);
            free(miss);
            r->rutes[trobat].alianca='S';
        } 
        else if(t->tipus==0x69){//nack
            //printf("Nack received from %s: %s\n", paraules[2], t->data);
            char *miss2;
            asprintf(&miss2, "Alliance with %s not established.\n", paraules[2]);
            printString(1, miss2);
            free(miss2);
        }
        
        }else{
             printf("Did not receive ack from %s\n", paraules[2]);
        }
        close(socket);
        free(t);
     
        return;
    }
    }else{
        printString(1, "No alliance request.\n");
        return;
    }
   
}

/*

DESCONEXIÓ DE MAESTER

*/
void notificarDesconnexioAliances(Regne *r){

    for(int i=0;i<r->n_rutes;i++){
        if(r->rutes[i].alianca=='S'){ //descomentar!!!!!!!!!!!!!!!!!!!!!
            Trama *trama = crearTrama( 0x27, r->ip, r->port, r->rutes[i].nomDest, 10, "DISCONNECT");
            enviarTramaDirecte(trama, r->rutes[i].portDest, r->rutes[i].ipDest, r->rutes[i].nomDest);//traam s'allibera aqui
            printf("Notified %s of disconnection.\n", r->rutes[i].nomDest);
            
        }
       /* if(strcmp(r->rutes[i].nomDest, "b") == 0){ //descomentar!!!!!!!!!!!!!!!!!!!!!
            Trama *trama = crearTrama( 0x27, r->ip, r->port, "b", 10, "DISCONNECT");
            enviarTramaDirecte(trama, 8111, "172.16.214.21", "a");//traam s'allibera aqui
            printf("Notified %s of disconnection.\n", r->rutes[i].nomDest);
            
        }*/
    }
}

/*
    PLEDGE STATUS
*/

__attribute__((unused)) static char *aliancesStr [3] = {"PENDING", "ALLIED", "ON_PENDING"};

void pledgeStatus (__attribute__((unused))Regne *r){

    /*for (int i = 0; i < r->n_rutes; i++){

        char *buffer;
        asprintf(&buffer, "- %s: %s\n", r->rutes[i].nomDest, aliancesStr[(int) r->rutes[i].alianca]);
        printString(1,buffer);
        free(buffer);
    }*/

    printString(1, "Command OK\n");
}

/*
    ENVOY STATUS
*/

void envoyStatus (__attribute__((unused)) Regne *r){

    printString(1, "Command OK\n");
}