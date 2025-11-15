
#include "terminal.h"

//crear conf harcoded
/*ClientConf *crearEnviamentArtificial(Regne *r) {
    Trama *trama = crearTrama(r, 0x01, "TheVale", "Loocoooo");
    ClientConf *clientConf = trobarSeguentSalt(r, trama); //pasar el regne per el routing i trama 
    printf("Creating artificial ClientConf to %s:%d\n", clientConf->ipDest, clientConf->portDest);
    if (!clientConf) {
        printString(1, "Error obtenint ClientConf\n");
        return NULL;
    }
    return clientConf;
}*/

/*
    Missatge d'entrada
*/
void iniciMs (Regne *r){

    char *buffer;
    asprintf(&buffer, "\nMaester of %s initialized. The board is set.\n", r->nom);
    printString(1,buffer);
    free(buffer);
}

/*
    EXIT
*/
void exitMs (Regne *r){

    char *buffer;
    asprintf(&buffer, "The Maester of %s signs off. The ravens rest.\n\n", r->nom);
    printString(1,buffer);
    free(buffer);
}

void runTerminal (Regne *r, char *fitxerStock){
    /*Regne *r = extractInfo (argv[1]);
    char *fitxerStock = argv[2];
    if (r == NULL) return;*/
    

    char comanda [COM_LEN];
    int comandaN = -1;

    iniciMs (r);

    while (comandaN != 7){ 
  
        printString (1, "$ ");
        read (0, comanda, COM_LEN);
        removeFirstEnter ( &comanda, COM_LEN);
        int n_paraules = 0;
        //printString (1, "\ntokenizing...\n");
        char **paraules = tokenizer(comanda, &n_paraules,' ');
        //printString (1, "\nprinting...\n");
        //printTokens(paraules, n_paraules);
        if (n_paraules == 0){
            freeTokens(paraules, n_paraules);
            continue;
        }
        //printString (1, "\ndone\n\n");
        comandaN = -1;
        if (n_paraules > 0) { 
            if (n_paraules >= 2) {
                
                if (strcasecmp (paraules[0],"LIST") == 0) {
                    if (strcasecmp (paraules[1],"REALMS") == 0) {
                        comandaN = 0;
                    } else if (strcasecmp (paraules[1],"PRODUCTS") == 0) {
                        comandaN = 3;
                    }  

                } else if (strcasecmp (paraules[0],"PLEDGE") == 0) {
                    if (strcasecmp (paraules[1],"RESPOND") == 0) {
                        comandaN = 1;
                    } else if (strcasecmp (paraules[1],"STATUS") == 0) {
                        comandaN = 5;
                    } else {
                        comandaN = 2;
                    }

                } else if (strcasecmp (paraules[0], "START") == 0 && strcasecmp (paraules[1], "TRADE") == 0) {
                    comandaN = 4;
                    
                } else if (strcasecmp (paraules[0], "ENVOY") == 0 && strcasecmp (paraules[1], "STATUS") == 0) {
                    comandaN = 6;
                }

            } else {

                if (strcasecmp(paraules[0],"EXIT") == 0){
                    comandaN = 7;
                } else {
                    comandaN = -1;
                }
            }
        }
        switch (comandaN){
            
            case -1:
                //comanda invalida
                printString (1, "Unknown command\n");
                break;
            case 0:
                if(n_paraules>2){
                    printString(1,"Unknown command\n");
                    break;
                } 
                listRealms (r);
                break;
            case 1:
                if(n_paraules>4){
                    printString(1,"Unknown command\n");
                    break;
                }   
                pledgeRespond (r, paraules, n_paraules);
                break;
            case 2:
                if (n_paraules < 2){
                    printString(1, "Did you mean to send a pledge? Please review syntax.\n");
                    break;
                } 
                if(n_paraules == 2){
                    //sortir de pledge , ha de ser de n_paraules >2
                    break;
                } 
                pledge (r, paraules);
                break;
            case 3:

                if(n_paraules == 3){
                    printString(1, "OK command\n");
                    break;
                }
                else if(n_paraules == 2){
                listProductsMaester(fitxerStock);
                }else{
                    printString(1, "unknown command\n");
                }
                break;
            case 4:
                //start trade
                if(n_paraules < 3){
                    printString(1, "Missing arguments, can’t start a trade. Please review the syntax.\n");
                    break;
                }
                if(n_paraules>3) {
                    printString(1,"Unknown command\n");
                    break;
                }  
                if(n_paraules==3){
                    if(!comprovarRutaRegneValid(paraules[2], r->n_rutes, r->rutes)){
                        printString(1,"That Realm cannot be found.\n");
                        break;
                    }
                }
                
                startTrade(r, comanda);
                break;
            case 5:
                if(n_paraules>2){
                    printString(1,"Unknown command\n");
                    break;
                }
                pledgeStatus (r);
                break;
            case 6:
                if(n_paraules>2){
                    printString(1,"Unknown command\n");
                    break;
                }
                envoyStatus (r);
                break;
            case 7:
                if(n_paraules>1){
                    printString(1,"Unknown command\n");
                    comandaN=-1;
                    break;
                }
                exitMs (r);
                break;
        }

        freeTokens(paraules, n_paraules);
    }

    freeRegne(r);
    return;

}