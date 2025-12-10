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

/*void sendEnvoy(Regne *r, char *casa){//no per aquesta fase

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
}*/

/*
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
}*/

void freeThreadPledge(Thread_pledge *tp) {
    if (!tp) return;
    if (tp->paraules) {
        for (int i = 0; i < tp->n_paraules; i++) {
            free(tp->paraules[i]);
        }
        free(tp->paraules);
    }
    free(tp);
}

void* pledge (void *arg){
    Thread_pledge *tp = (Thread_pledge *) arg;
    Regne *r = tp->r;
    char **paraules = tp->paraules;

   
    if (!comprovarRegneExisteix(r, paraules[1])){
           printString(1, "That Realm cannot be found , remember its case sensitive.\n");
            freeThreadPledge(tp);
           return NULL;
    }
    char * filename=malloc(FILENAME_SIGIL_SIZE*sizeof(char));
    snprintf(filename, FILENAME_SIGIL_SIZE, "segells/%s",paraules[2]);
    int fd=open(filename, O_RDONLY);
    if(fd==-1){
        printString(1, "Error opening  file\n");
        
        free(filename);
        freeThreadPledge(tp);
       return NULL;
    }
    int indexRuta=-1;
    for(int i=0;i<r->n_rutes;i++){
        
        if(strcmp(r->rutes[i].nomDest,paraules[1])==0){
            if( r->rutes[i].alianca=='S'){
                printString(1, "You are already allied with this Realm.\n");
                close(fd);
                 freeThreadPledge(tp);
                    free(filename);
                return NULL;
            }
            if( r->rutes[i].alianca=='P'){
                printString(1, "You have already sent an alliance request to this Realm.\n");
                close(fd);
                 freeThreadPledge(tp);
                    free(filename);
                return NULL;
            }
            if( r->rutes[i].alianca=='O'){
                printString(1, "You have a pending pledge from that Maester.\n");
                close(fd);
                freeThreadPledge(tp);
                free(filename);
                return NULL;
            }
            
            r->rutes[i].alianca='P';
            indexRuta=i;
            //printString(1, "Alliance request sent.\n");
            break;
        }
     }
        resetEnvoy(&r->envoys[tp->indexEnvoy]);
        setStatusEnvoy(&r->envoys[tp->indexEnvoy], "pledge");
        char *pathPNG;
        asprintf(&pathPNG, "%s/%s_sigil.png", r->rutes[indexRuta].nomDest, r->nom);
        int fdsigil = open(pathPNG, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fdsigil < 0) {
            free(pathPNG);
            close(socket);
            return;
        }

        close(fdsigil); // tancar ara, després obriràs per escriure trames
        free(pathPNG);

    char data[DATA_SIZE];
    int size=getSize(fd);
    char *md5 = calcularMD5SUM(filename);
    printf("MD5: %s\n", md5);
    if (!md5) {
        printString(1, "Error calculant MD5\n");
        //freeTokens(paraules, n_paraules);
        close(fd);
        free(filename);
        freeThreadPledge(tp);
        return NULL;
    }
    snprintf(data, DATA_SIZE, "%s&%s&%d&%s",r->nom,paraules[2],size, md5);
    Trama *trama = crearTrama( 0x01, r->ip, r->port, paraules[1], strlen(data), data);
    enviarTramaDinamica(trama, r, 0);//trama s'allibera aqui
    close(fd);
    free(filename); 
    free(md5);
    char *buffer;
        asprintf(&buffer, "Pledge sent to %s. Envoy 1 is on its way.\n", paraules[1]);//tambe s'ha de posar quin envoy
        printString(1, buffer);
        free(buffer);

    r->envoyTemporalIsBusy = 1;

    int startPos=0;
        char * filenameSigil=malloc(FILENAME_SIGIL_SIZE*sizeof(char));
        snprintf(filenameSigil, FILENAME_SIGIL_SIZE, "segells/%s.png",r->nom);
        int fdSigil=open(filenameSigil, O_RDONLY);
        if(fdSigil==-1){
            printString(1, "Error opening  file to read sigil\n");
            free(filenameSigil);
            freeThreadPledge(tp);
            return NULL;
        }


        
         char *sigilData;
        printString(1, "Sending sigil ...\n");
        
        
        key_t semKey = generarClauIPC(r->nom, r->rutes[indexRuta].nomDest, 1);
        semaphore semPledge;
        SEM_constructor_with_name(&semPledge, semKey);
        SEM_init(&semPledge, 1);


        while(startPos<=size){
            SEM_wait(&semPledge);
           // usleep(50000);
            if (r->exit && *(r->exit)) {
                  close(fdSigil);
                    free(filenameSigil);
                    freeThreadPledge(tp);
                    return NULL;
            }
            if(startPos+275>size){
                
                
                sigilData=readFd(fdSigil,startPos,size-startPos);
                trama = crearTrama(0x02, r->ip, r->port, r->rutes[indexRuta].nomDest , size-startPos,sigilData);

            }else{
                sigilData=readFd(fdSigil,startPos,275);
                trama = crearTrama(0x02, r->ip, r->port, r->rutes[indexRuta].nomDest , 275,sigilData);
            }
             //utiltizar semafor wait
             //SEM_wait(semPledge);
            int socket=enviarTramaDinamica(trama, r, 0);
            if (socket<0){
                printString(1, "Error sending sigil trama\n");
                free(sigilData);
                close(fdSigil);
                free(filenameSigil);
                freeThreadPledge(tp);

                return NULL;
            }
            startPos+=275;

            free(sigilData);
            
        }
        printString(1, "Sigil sent.\n");

       
        close(fdSigil);
        free(filenameSigil);


    time_t inici = time(NULL);         
    const double timeout = 120.0;
    while(1){
        sleep(1);
        time_t ara = time(NULL);
        double diff = difftime(ara, inici);
        if (diff >= timeout) {                      
            char *buffer;
            asprintf(&buffer, "Pledge to %s timed out.\n", r->rutes[indexRuta].nomDest);
            printString(1, buffer);
            free(buffer);
            r->rutes[indexRuta].alianca='T';//T de timeout
            break;
        }
        if (r->exit && *(r->exit)) {
            break;
        }
        /*if(r->rutes[indexRuta].alianca=='S' || r->rutes[indexRuta].alianca=='N'){
            break;
        }*/
        if(r->envoys[tp->indexEnvoy].ackFitxer==1 && r->envoys[tp->indexEnvoy].ackMD5SUM==1){//ha rebut acks
            break;
        }
    }
    //r->envoys[tp->indexEnvoy].status=NULL;
    resetEnvoy(&r->envoys[tp->indexEnvoy]);
    freeThreadPledge(tp);
    return NULL;
} 



                
/*
    PLEDGE RESPOND
*/


void pledgeRespond ( Regne *r,char** paraules,int n_paraules){

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

    if(r->rutes[trobat].ackFitxerStatus==0 || r->rutes[trobat].ackMD5SUMStatus==0){
        r->rutes[trobat].ackFitxerStatus=0;
        r->rutes[trobat].ackMD5SUMStatus==0;
        r->rutes[trobat].alianca='N';
        printString(1,"Something went wrong, ack's are wrong. Try again when next pledge.\n");
        return;
    }

    if(r->rutes[trobat].alianca=='O'){
        char* data[DATA_SIZE];
     if(strcasecmp(paraules[3], "reject")==0){
        //<ACCEPT/REJECT>&<RealmName>
        snprintf(data, DATA_SIZE, "REJECT&%s",r->nom);
        enviarTramaDirecte(crearTrama(0x03, r->ip, r->port, r->rutes[trobat].nomDest, strlen(data), data), r->rutes[trobat].portDest, r->rutes[trobat].ipDest, paraules[2]);
        r->rutes[trobat].alianca='N';
        return;
    } else if(strcasecmp(paraules[3], "accept")==0){
        snprintf(data, DATA_SIZE, "ACCEPT&%s",r->nom);
        int socket=enviarTramaDirecteSenseTancar(crearTrama(0x03, r->ip, r->port, r->rutes[trobat].nomDest, strlen(data), data), r->rutes[trobat].portDest, r->rutes[trobat].ipDest, paraules[2]);
        if(socket<0){
            return;
        }
        //Trama * t = waitForRead(2.0,socket,r->exit);
        Trama * t = malloc(sizeof(Trama));
        read(socket, t, sizeof(Trama));
        if(t!=NULL){
        //si es ack o nack
        if(t->tipus==0x31){//ack
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
             char *miss3;
            asprintf(&miss3, "Did not receive ack from %s\n", paraules[2]);
            printString(1, miss3);
            free(miss3);
        }
        close(socket);
        free(t);
     
        return;
    }else{
        printString(1, "Please respond with accept or reject.\n");
        return;
    }
    }else{
        printString(1, "No alliance request.\n");
        return;
    }
   
}


void pledgeStatus(Regne *r){

    int foundAny=0;
    for (int i = 0; i < r->n_rutes; i++){
        if(strcmp(r->rutes[i].nomDest,"DEFAULT")==0) continue;

        printString(1, "- ");
        printString(1, r->rutes[i].nomDest);
        printString(1, ": ");
        switch (r->rutes[i].alianca){
            case 'O':
                printString(1, "ON_PENDING\n");
                foundAny=1;
                break;
            case 'S':
                printString(1, "ALLIED\n");
                foundAny=1;
                break;
            case 'P':
                printString(1, "PENDING\n");
                foundAny=1;
                break;
            case 'T':
                printString(1, "FAILED\n");
                foundAny=1;
                break;
        }  
    }
    if(foundAny==0){
        printString(1, "You have no pledges awaiting or accepted\n");
    }
}

/*

DESCONEXIÓ DE MAESTER

*/
void notificarDesconnexioAliances(Regne *r){

    for(int i=0;i<r->n_rutes;i++){
        if(r->rutes[i].alianca=='S'){ 
            Trama *trama = crearTrama( 0x27, r->ip, r->port, r->rutes[i].nomDest, 10, "DISCONNECT");
            enviarTramaDirecte(trama, r->rutes[i].portDest, r->rutes[i].ipDest, r->rutes[i].nomDest);//traam s'allibera aqui
            char *miss;
            asprintf(&miss, "Notified %s of disconnection.\n", r->rutes[i].nomDest);
            printString(1, miss);
            free(miss);
            
        }
    }
}



/*
    ENVOY STATUS
*/

void envoyStatus (__attribute__((unused)) Regne *r){

    printString(1, "Command OK\n");
}