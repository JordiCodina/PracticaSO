
#include "terminal.h"
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
    asprintf(&buffer, "\nThe Maester of %s signs off. The ravens rest.\n\n", r->nom);
    printString(1,buffer);
    free(buffer);
}

/*

@brief Funció que gestiona la terminal del regne.
@param r Punter a la estructura Regne que conté la informació del regne actual.
@param fitxerStock Cadena de caràcters que conté el nom del fitxer d'stock de productes.

*/

void runTerminal (Regne *r, char *fitxerStock){
    char *comanda = NULL;
    int comandaN = -1;
    creaServer(r);
    if (r->fd_server < 0) {
        return;
    }
    
    fd_set master_set;/*, read_set;*/
    FD_ZERO(&master_set);
    FD_SET(r->fd_server, &master_set);
    FD_SET(STDIN_FILENO, &master_set);
    int max_fd = (r->fd_server > STDIN_FILENO) ? r->fd_server : STDIN_FILENO;
    iniciMs (r);
    printString(1, "$");
    while (comandaN != 7){ 
        if(*(r->exit) == 1){
            comandaN = 7;
            break;
        }
        //escolta de comandes per teclat, fd de servidor (accept) i sockets de servidor(read)
        //si no detecta cap comanda pero si altres, retorna null
        comanda = escoltarTot(&master_set, &max_fd, r);
        if (comanda == NULL) {
            continue;
        }
            int n_paraules = 0;
            char **paraules = tokenizer(comanda, &n_paraules,' ');
            if (n_paraules == 0){
                freeTokens(paraules, n_paraules);
                free(comanda);
                printString(1, "$");
                continue;
            }
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
            
            if(*(r->exit) == 1){
                comandaN = 7;
                break;
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
                    if (n_paraules < 3){
                        printString(1, "Did you mean to send a pledge? Please review syntax.\n");
                        break;
                    } 

                    if(r->envoys[0].status==NULL){
                        Thread_pledge *tp = malloc(sizeof(Thread_pledge));
                        tp->r = r;
                        tp->n_paraules = n_paraules;
                        tp->paraules = malloc(n_paraules * sizeof(char *));
                        tp->indexEnvoy = 0; 
                        for (int i = 0; i < n_paraules; i++) {
                            tp->paraules[i] = strdup(paraules[i]);
                        }
                        
                        afegirThread(r, comandaN);
                        pthread_create(&r->fils[r->n_fils - 1].thread_id, NULL, pledge, (void *)tp);
                    // pledge (r, paraules);
                    }else{
                        printString(1, "Only one pledge may be sworn at a time. Pledge has been withdrawn\n");
                    }
                    
                    break;
                case 3:

                    if(n_paraules == 3){
                        listProductsAliat(r, paraules[2]/*,r->exit*/);
                        break;
                    }
                    else if(n_paraules == 2){
                        listProductsMaester(fitxerStock,0,"Maester");
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
                    
                    startTrade(r, comanda, &master_set, &max_fd);
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
                
                    break;
            }
            if(comandaN != 7){
                printString(1, "$");
            }
            freeTokens(paraules, n_paraules);
            free(comanda);
            comanda = NULL;
            joinThreadServidor(r);
                                  
    }
    exitMs (r);
    notificarDesconnexioAliances(r);
    *(r->exit)=1;
    return;

}

