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

void pledge (__attribute__((unused)) Regne *r, __attribute__((unused)) char comanda [100]){

    char regneTrobat = 0;

    for (int i = 0; i < r->n_rutes; i++){

        int nomRegLen = strlen(r->rutes[i].nomDest);

        if (!substrCmp(comanda, 7, 7 + nomRegLen, r->rutes[i].nomDest)){
            //sendEnvoy(r, r->rutes[i].nomDest);
            printString(1, "Command OK\n");
            regneTrobat = 1;
        }
    }

    if (!regneTrobat){
        printString(1, "That Realm cannot be found.\n");
    }
}

/*
    PLEDGE RESPOND
*/

void pledgeRespond (__attribute__((unused)) Regne *r){

    printString(1, "Command OK\n");
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