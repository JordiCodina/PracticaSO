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

void sendEnvoy(Regne *r, char *casa){

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


void pledge (Regne *r,char **paraules){

    if (!comprovarRegneExisteix(r, paraules[1])){
           printString(1, "That Realm cannot be found , remember its case sensitive.\n");
           return;
    }
    for(int i=0;i<r->n_rutes;i++){
        if(strcmp(r->rutes[i].nomDest,paraules[1])==0){
            r->rutes[i].alianca='P';
            //printString(1, "Alliance request sent.\n");
            break;
        }
    }
    char data[DATA_SIZE];
    snprintf(data, DATA_SIZE, "%s&%s&%d&%s",r->nom,paraules[2],13545,"MDSUM5");
    Trama *trama = crearTrama( 0x01, r->ip, r->port, paraules[1], strlen(data), data);
    enviarTrama(trama, r, 0);

    char *buffer;
        asprintf(&buffer, "Pledge sent to %s.\n", paraules[1]);//tambe s'ha de posar quin envoy
        printString(1, buffer);
        free(buffer);
    free(trama);
    //free(clientConf);
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

    if(r->rutes[trobat].alianca=='P'){
     if(strcasecmp(paraules[3], "reject")==0){
        respondPledge(0);
        r->rutes[trobat].alianca='N';
        return;
    }
     if(strcasecmp(paraules[3], "accept")==0){
        respondPledge(1);
        r->rutes[trobat].alianca='S';
        return;
    }
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