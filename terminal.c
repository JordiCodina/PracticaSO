
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
    asprintf(&buffer, "\nThe Maester of %s signs off. The ravens rest.\n\n", r->nom);
    printString(1,buffer);
    free(buffer);
}


void *waitExit(void *arg) {
    ThreadReadTerminal *tl = (ThreadReadTerminal *) arg;

    tl->comanda = malloc(COM_LEN);  
    if (!tl->comanda) {
        tl->thread_complete = 1;
        return NULL;
    }
    int n = read(0, tl->comanda, COM_LEN - 1);
    if (n < 0) n = 0;

    if (n > 0 && tl->comanda[n-1] == '\n') {
        tl->comanda[n-1] = '\0';
    } else {
        tl->comanda[n] = '\0';
    }
    tl->thread_complete = 1;
    return NULL;
}


char *waitForExit(int *exitflag) {

    ThreadReadTerminal *tl = malloc(sizeof(ThreadReadTerminal));
    tl->thread_complete = 0;
    pthread_t waitReadThread;
    pthread_create(&waitReadThread, NULL, waitExit, tl);
    while (1) {
        if (*exitflag == 1) {
            pid_t pid = fork();
            if (pid == 0) {

                pthread_kill(waitReadThread, SIGKILL);
                _exit(0);
            }
            free(tl->comanda);   
            free(tl);
            return NULL;
        }

        if (tl->thread_complete == 1) {
            break;
        }
        usleep(10000);
    }
    pthread_join(waitReadThread, NULL);
    char *result = strdup(tl->comanda);
    free(tl->comanda);  
    free(tl);
    return result;
}

#define BUF_SIZE 256

char* llegirComanda(int *exitflag) {
    if (*exitflag == 1) {
        return strdup("controlc");
    }
    int flags = fcntl(1, F_GETFL, 0);
    fcntl(1, F_SETFL, flags | O_NONBLOCK);
    char buf[BUF_SIZE];
    while (1) {
        if (*exitflag == 1) {
            return strdup("controlc");
        }
        int n = read(1, buf, BUF_SIZE - 1);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(100000);
                continue;
            } else {
                return NULL;
            }
        } else if (n == 0) {
            return NULL;
        } else {
            buf[n] = '\0';
            size_t len = strlen(buf);
            if (len > 0 && buf[len-1] == '\n') {
                buf[len-1] = '\0';
            }
            return strdup(buf);
        }
    }
}

void runTerminal (Regne *r, char *fitxerStock,int* exitflag){
    
    char *comanda = NULL;
    int comandaN = -1;

    iniciMs (r);


    while (comandaN != 7){ 
      
        if(*exitflag == 1){
            comandaN = 7;
            break;
        }
        printString (1, "$ ");
        //comanda = waitForExit(exitflag);
        comanda = llegirComanda(exitflag);

        if (comanda == NULL) {
            // EOF o error
        } else if (strcmp(comanda, "controlc") == 0) {
            // senyal de sortida
            comandaN = 7;
            free(comanda);
            break;
        }
    
       //sleep(1); //per evitar problemes de concurrencia
        if (comanda == NULL) {
            comandaN = 7; 
            free(comanda);  
            break;
        }
     
       // removeFirstEnter ( &comanda, strlen(comanda) );
        int n_paraules = 0;
        //printString (1, "\ntokenizing...\n");
        char **paraules = tokenizer(comanda, &n_paraules,' ');
        //printString (1, "\nprinting...\n");
        //printTokens(paraules, n_paraules);
        if (n_paraules == 0){
            freeTokens(paraules, n_paraules);
            free(comanda);
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
        
        if(*exitflag == 1){
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
                pledgeRespond (r, paraules, n_paraules,exitflag);
                break;
            case 2:
                if (n_paraules < 3){
                    printString(1, "Did you mean to send a pledge? Please review syntax.\n");
                    break;
                } 
                pledge (r, paraules);
                break;
            case 3:

                if(n_paraules == 3){
                    listProductsAliat(r, paraules[2],exitflag);
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
               
                break;
        }
         
       

        freeTokens(paraules, n_paraules);
        free(comanda);
        comanda = NULL;
    }

    exitMs (r);
    notificarDesconnexioAliances(r);
    //freeRegne(r);
    *exitflag=1;
    //sleep(1);
    

    return;

}