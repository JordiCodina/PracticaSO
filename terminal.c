
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


void *waitExit(void *arg) {//no sutilitza aquesta funcio 
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


char *waitForExit(int *exitflag) {//no s'utlitza aquesta funcio pero la deixem per si de cas

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

void afegirthreadServidor(Regne *r, ServerThread *st) {
    if (r->st == NULL) {
        r->st = malloc(sizeof(ServerThread*));
        r->n_serverThreads = 1;
    } else {
        r->st = realloc(r->st, (r->n_serverThreads + 1) * sizeof(ServerThread*));
        r->n_serverThreads++;
    }
    r->st[r->n_serverThreads - 1] = st; 
}
void removeThreadServidor(Regne *r, int index) {
    if (r->n_serverThreads <= 0 || index < 0 || index >= r->n_serverThreads) return;

    free(r->st[index]); 
    for (int i = index; i < r->n_serverThreads - 1; i++) {
        r->st[i] = r->st[i + 1];
    }
    r->n_serverThreads--;
    if (r->n_serverThreads == 0) {
        free(r->st);
        r->st = NULL;
    } else {
        r->st = realloc(r->st, r->n_serverThreads * sizeof(ServerThread*));
    }
}
void afegirThread(Regne *r, int comandaN) {
    if (r->fils == NULL) {
        r->fils = malloc(sizeof(Fil));
        r->n_fils = 1;
    } else {
        r->fils = realloc(r->fils, (r->n_fils + 1) * sizeof(Fil));
        r->n_fils++;
    }
    r->fils[r->n_fils - 1].comanda = comandaN;
}

void joinThreadServidor(Regne *r) {
     for(int i=0;i<r->n_serverThreads;){
        if(r->st[i]->thread_complete==1){
            close(r->st[i]->client_fd);
            pthread_join(r->st[i]->thread_id, NULL);
            removeThreadServidor(r, i); 
        } else {
            i++; 
        }
    }
}

void handleNewConnection(Regne *r, fd_set *master_set, int *max_fd) {
    int new_socket = accept_client_connection(r->fd_server);
    if (new_socket < 0) {
        printString(1, "Error acceptant connexió de client\n");
        return;
    }
    FD_SET(new_socket, master_set);
    if (new_socket > *max_fd) *max_fd = new_socket;
    ServerThread *st = malloc(sizeof(ServerThread));
    st->client_fd = new_socket;
    st->thread_complete = 0;
    afegirthreadServidor(r, st);
}

void readSockets(fd_set *fds_set, Regne *r, int *max_fd) {
    //accept()
    if (FD_ISSET(r->fd_server, fds_set)) {
        handleNewConnection(r, fds_set, max_fd);
    }
    //read de clients
    for (int i = 0; i < r->n_serverThreads; i++) {
        if (FD_ISSET(r->st[i]->client_fd, fds_set)) {
            if (r->st[i]->thread_complete == 0) {
                ThreadUnServidor *tus = malloc(sizeof(ThreadUnServidor));
                tus->r = r;
                tus->st = r->st[i];

                if (pthread_create(&r->st[i]->thread_id, NULL, runServer, (void *)tus) != 0) {
                    printString(1, "Error creant thread\n");
                    free(tus);
                }
            }
        }
    }
}


char * escoltarTot(fd_set master_set, int max_fd, Regne *r) {


    char *comanda = NULL;
    fd_set read_set;
    read_set = master_set;  
        struct timeval timeout = {1, 0}; //segons , microsegons
        int select_response = select(max_fd + 1, &read_set, NULL, NULL, &timeout);
        if (select_response < 0) {
            return NULL;//!!!!
        }
        if (select_response == 0) {//timeout
            joinThreadServidor(r);
            return NULL; 
        }
        readSockets(&read_set, r, &max_fd);
        // fd=0
        if (FD_ISSET(STDIN_FILENO, &read_set)) {
            comanda = malloc(COM_LEN);
            int n = read(STDIN_FILENO, comanda, COM_LEN - 1);
            if (n < 0) {

                printString(1, "$");
                free(comanda);
                return NULL; 
            }

            if (n > 0 && comanda[n - 1] == '\n') comanda[n - 1] = '\0'; 
            comanda[n] = '\0';
            size_t len = strlen(comanda);
            if (len > 0 && comanda[len-1] == '\n') {
                comanda[len-1] = '\0';
            }
            
             return comanda;
        }
    return NULL;
   

}
void runTerminal (Regne *r, char *fitxerStock){
    char *comanda = NULL;
    int comandaN = -1;
    creaServer(r);
    if (r->fd_server < 0) {
        return;
    }
    
    fd_set master_set, read_set;
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
        comanda = escoltarTot(master_set, max_fd, r);
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

                    if(r->envoyTemporalIsBusy == 0){
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
                        printString(1, "Envoy is busy.Try again later\n");
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
                    
                    startTrade(r, comanda, master_set, max_fd);
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
    //freeRegne(r); //es fa al maester.c
    *(r->exit)=1;
    return;

}

