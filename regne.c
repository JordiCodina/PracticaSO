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


int obrirFitxerSigil(char *rutaSegell) {
    char *filenameSigil = malloc(FILENAME_SIGIL_SIZE * sizeof(char));
    if (!filenameSigil) return -1;
    snprintf(filenameSigil, FILENAME_SIGIL_SIZE, "segells/%s", rutaSegell);
    int fdSigil = open(filenameSigil, O_RDONLY);
    if (fdSigil == -1) {
        printString(1, "Error opening sigil file\n");
        free(filenameSigil);
        return -1;
    }

    free(filenameSigil);
    return fdSigil;
}

                
/*
    PLEDGE RESPOND
    @brief Funció que gestiona la comanda pledge respond

    @param r Punter a la estructura Regne que conté la informació del regne actual.
    @param paraules Matriu de cadenes que conté els arguments de la comanda.
    @param n_paraules Nombre d'elements a la matriu de cadenes.

    
*/

void pledgeRespond(Regne *r, char **paraules, int n_paraules) {

    if (paraules == NULL || n_paraules < 4) {
        printString(1, "Please specify the Realm and response (accept/reject).\n");
        return;
    }
    //trobar la ruta corresponent
    int trobat = -1;
    for (int i = 0; i < r->n_rutes; i++) {
        if (strcmp(r->rutes[i].nomDest, paraules[2]) == 0) {
            trobat = i;
            break;
        }
    }
    if (trobat < 0) {
        printString(1, "That Realm cannot be found.\n");
        return;
    }

    if (r->rutes[trobat].alianca == 'S') {
        printString(1, "You are already allied with this Realm.\n");
        return;
    }

    if (r->rutes[trobat].alianca == 'N') {
        printString(1, "No alliance request.\n");
        return;
    }
    


    if (r->rutes[trobat].ackFitxerStatus == 0 || r->rutes[trobat].ackMD5SUMStatus == 0) {
        //printf("ackFitxerStatus: %d , ackMD5SUMStatus: %d\n",r->rutes[trobat].ackFitxerStatus,r->rutes[trobat].ackMD5SUMStatus);
        r->rutes[trobat].ackFitxerStatus = 0;
        r->rutes[trobat].ackMD5SUMStatus = 0;
        r->rutes[trobat].alianca = 'N';
        printString(1, "Something went wrong, ack's are wrong. Try again when next pledge.\n");
        
        return;
    }
    if (r->rutes[trobat].alianca == 'O') {
        char data[DATA_SIZE];
        if (strcasecmp(paraules[3], "reject") == 0) {

            snprintf(data, DATA_SIZE, "REJECT&%s", r->nom);

            // envia i tanca internament
            Trama *t = crearTrama(0x03, r->ip, r->port,r->rutes[trobat].nomDest, strlen(data), data);

            enviarTramaDirecte(t,
                               r->rutes[trobat].portDest,
                               r->rutes[trobat].ipDest,
                               paraules[2]);

            r->rutes[trobat].alianca = 'N';
            return;
        }
        else if (strcasecmp(paraules[3], "accept") == 0) {

            //enviem trama 0x03 amb accept
            snprintf(data, DATA_SIZE, "ACCEPT&%s", r->nom);


            //EL PING PONG  serveix per comprovar que l'aliat esta actiu abans d'establir l'aliança , ja que sino dona errros més endavant
            //ho he fet a la fase 4

            //PING
            Trama* ping = crearTrama(0x26, r->ip, r->port, r->rutes[trobat].nomDest, 0, NULL);
            int socketPing=enviarTramaDirecteSenseTancar(ping, r->rutes[trobat].portDest, r->rutes[trobat].ipDest, r->rutes[trobat].nomDest);//sallibera trama ping aqui
            //PONG
            Trama * pong = crearTrama(0x00,NULL,0,NULL,0,NULL);
            int n1 = read(socketPing, pong, sizeof(Trama));
            close(socketPing);
            if (n1 <= 0 || pong->tipus != 0x26) {
                free(pong);
                printString(1, "Did not receive PONG from ally. Alliance cannot be established.\n");
                r->rutes[trobat].alianca = 'N';
                return; 
            }  
            free(pong);
            
            
            Trama *t_enviar = crearTrama(0x03, r->ip, r->port,r->rutes[trobat].nomDest, strlen(data), data);
            enviarTramaDirecte(t_enviar,r->rutes[trobat].portDest,r->rutes[trobat].ipDest,paraules[2]);
 
            char *miss;
            asprintf(&miss, "Alliance with %s established.\n", paraules[2]);
            printString(1, miss);
            free(miss);

            r->rutes[trobat].alianca = 'S';
            return;
        }

        else {
            printString(1, "Please respond with accept or reject.\n");
            return;
        }
    }
    else {
        printString(1, "No alliance request.\n");
        return;
    }
} 



void pledgeStatus(Regne *r){

    int foundAny=0;
    for (int i = 0; i < r->n_rutes; i++){
        if(strcmp(r->rutes[i].nomDest,"DEFAULT")==0 || r->rutes[i].alianca=='N' || r->rutes[i].alianca=='I') continue;
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

void envoyStatus (Regne *r){

    for (int i = 0; i < r->n_envoys; i++){
        char *buffer;
        if (r->envoys[i].status == 1){
            asprintf(&buffer, "- Envoy %d: ON MISSION to %s\n", i + 1, r->envoys[i].nomRegneViatge);
        }
        else {
            asprintf(&buffer, "- Envoy %d: FREE\n", i + 1);
        }
        printString(1, buffer);
        free(buffer);
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
    st->thread_complete = THREAD_INACTIVE;
    afegirthreadServidor(r, st);
}

void readSockets(fd_set *fds_set, Regne *r, int *max_fd) {
    //accept()
    if (FD_ISSET(r->fd_server, fds_set)) {
        usleep(500);
        handleNewConnection(r, fds_set, max_fd);
    }
    //read de clients
    //usleep(1000);
    for (int i = 0; i < r->n_serverThreads; i++) {
        if (FD_ISSET(r->st[i]->client_fd, fds_set)) {
             
            //printf("Client fd: %d, Thread complete: %d\n", r->st[i]->client_fd,r->st[i]->thread_complete);
            if (r->st[i]->thread_complete == THREAD_INACTIVE) {
                
                ThreadUnServidor *tus = malloc(sizeof(ThreadUnServidor));
                tus->r = r;
                tus->st = r->st[i];
                r->st[i]->thread_complete = THREAD_RUNNING;
                usleep(4500);
                //usleep(3000);
                if (pthread_create(&r->st[i]->thread_id, NULL, runServer, (void *)tus) != 0) { 
                    r->st[i]->thread_complete =THREAD_DONE ;
                    printString(1, "Error creant thread\n");
                    free(tus);
                }
            }
        }
    } 
}

void processarMissatgeEnvoy(Regne *r, EnvoyWritesRegne *msg) {

    int indexRuta = msg->indexRuta;
    if (indexRuta == -1) {
        printString(1, "Route not found for the specified Realm.\n");
        return;
    }
    switch (msg->comanda) {
        case 1://pledge
            if(msg->statusPledge==1){//accpet
                
                if (r->rutes[indexRuta].alianca == 'T') {
                        break;
                }
                r->rutes[indexRuta].alianca = 'S';

                //actualitza port
                r->rutes[indexRuta].portDest = msg->portDest;
                //actualitza ip
                free(r->rutes[indexRuta].ipDest);
                r->rutes[indexRuta].ipDest = strdup(msg->ipDest); 

                char *buffer;
                asprintf(&buffer, ">>> Alliance with %s forged successfully!\n", r->rutes[indexRuta].nomDest);
                printString(1, buffer);
                free(buffer);
            }else if(msg->statusPledge==2){ //reject

                printString(1, "Pledge rejected by the other Realm.\n");
                r->rutes[msg->indexRuta].alianca = 'N'; 

            }else if (msg->statusPledge==0){//timeout 
                r->rutes[msg->indexRuta].alianca = 'T'; 
                char *tmp = NULL;
                if (asprintf(&tmp, "Pledge to %s timed out.\n",  r->rutes[indexRuta].nomDest) != -1) {
                    printString(1, tmp);
                    free(tmp);
                }
                r->envoys[msg->indexEnvoy].status = 0;
                return;
            }
            
            break;

           
        case 2://list products
             //deixo aaquest case per si de cas volem ficar algun printf o algo més endavant
            break;

        case 3://send 
             //deixo aaquest case per si de cas volem ficar algun printf o algo més endavant
            break;
        case 4://probelma (acks,conexions, etc) , simplemetn cancelar la missio
                r->rutes[indexRuta].alianca = 'N';
                printString(1,"Something went wrong. Pledge has failed\n");
        break;
        case 5:
        //aqui es quan falla list products o send trade
                printString(1,"Something went wrong.Try again\n");
        break;
    }
    setEnvoyLliure(r, msg->indexEnvoy);
}

void readEnvoys(fd_set *fds_set, Regne *r) {
    for (int i = 0; i < r->n_envoys; i++) {
        int fd = r->envoys[i].pipeRead;
        if (FD_ISSET(fd, fds_set)) {
            EnvoyWritesRegne msg;
           // char buffer[275];
            int n = read(fd, &msg, sizeof(EnvoyWritesRegne));
            
            if (n <= 0) {
                close(fd);
                continue;
            }
            processarMissatgeEnvoy(r, &msg);
        }
    }
}

char * escoltarTot(fd_set *master_set, int *max_fd, Regne *r) {
    char *comanda = NULL;
    fd_set read_set = *master_set; 
        struct timeval timeout = {1, 0}; //segons , microsegons
        int select_response = select(*max_fd + 1, &read_set, NULL, NULL, &timeout);

        if (select_response < 0) {
            return NULL;
        }
        if (select_response == 0) {//timeout
            joinThreadServidor(r);
            return NULL; 
        }
        readSockets(&read_set, r, max_fd);
        readEnvoys(&read_set, r);
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

int pledgeComprovar(Regne *r, char **paraules){

    if (!comprovarRegneExisteix(r, paraules[1])) {
        printString(1, "No such realm exists. The pledge is hereby withdrawn. Remember is case sensitive.\n");
        return -1;
    }
    int indexRuta = -1;
    for (int i = 0; i < r->n_rutes; i++) {
        if (strcmp(r->rutes[i].nomDest, paraules[1]) == 0) {
            if (r->rutes[i].alianca == 'S') {
                printString(1, "You are already allied with this Realm.\n");

                return -1;
            }
            if (r->rutes[i].alianca == 'P') {
                printString(1, "You have already sent an alliance request to this Realm.\n");
 
                return -1;
            }
            if (r->rutes[i].alianca == 'O') {
                printString(1, "You have a pending pledge from that Maester.\n");
            
                return -1;
            }

             
            indexRuta = i;
            break;
        }
    }
    if (indexRuta == -1) {
        printString(1, "Route not found for this Realm.\n");
        return -1;
    }

    crearDirectori(r->rutaArxius);

    return indexRuta;
}


//el regne esta desactualitzat !!! nome
int pledgeEnvoyTramaInicial(char filenameSigil[30] , char nomDesti[50], Regne *r){

   char *filename;
    asprintf(&filename, "segells/%s", filenameSigil);
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        printString(1, "Sigil doesn't exist\n");
        free(filename);
        return 0;
    }

    char data[DATA_SIZE];
    int size = getSize(fd);
    char *md5 = calcularMD5SUM(filename);
    if (!md5) {
        printString(1, "Error calculant MD5\n");
        close(fd);
        free(filename);

        return 0;
    }
    snprintf(data, DATA_SIZE, "%s&%s&%d&%s", r->nom, filename, size, md5);

    free(filename);
    free(md5);
    close(fd);
     Trama *trama = crearTrama(0x01, r->ip, r->port,nomDesti , strlen(data), data);
    int socket = enviarTramaDinamica(trama, r, 0); // trama s'allibera aquí
    if (socket < 0) {
        printString(1, "Cannot send pledge request.\n");
        return 0;
    }

    char *buffer = NULL;
    if (asprintf(&buffer, "Pledge sent to %s. Envoy 1 is on its way.\n", nomDesti) != -1) {
        printString(1, buffer);
        free(buffer);
    }
    return 1;
}
int pledgeEnvoyEnviarSigil(char filenameSigil[30] , char nomDesti[50], Regne *r,int *exitflag){

    int startPos = 0;
    char *filename;
    asprintf(&filename, "segells/%s", filenameSigil);
    int fdSigil = open(filename, O_RDONLY);
    if (fdSigil == -1) {
        printString(1, "Sigil doesn't exist\n");
        free(filename);
        return -1;
    }

    Trama *trama;
    int size = getSize(fdSigil);
    usleep(100000);
    printString(1, "Sending sigil ...\n");
    while (startPos <= size) {   
        usleep(10000);
        //usleep(1000);
        if (*exitflag) {
            close(fdSigil);
            return 0;
        }
        int chunk = (startPos + 275 > size) ? (size - startPos) : 275;//si supera el size ajustem el chunk ja que la ultima trama  no sera 275
        if (chunk <= 0) break;
        char *sigilData = readFd(fdSigil, startPos, chunk);//llegim el fitxer del segell marcat per el if anterior(chunk pot ser mes petit que 275 o 275 maxim)
        if (!sigilData) {
            printString(1, "Error reading sigil data\n");
            close(fdSigil);
            return -1;
        }
        //crear i enviar la trama 0x02 per hops , es la trama per enviar el segell trama a trama
        trama = crearTrama(0x02, r->ip, r->port,nomDesti , chunk, sigilData);
        int socket = enviarTramaDinamica(trama, r, 0);

        if (socket < 0) {
            printString(1, "Error sending sigil trama.\n");
            free(sigilData);
            close(fdSigil);
            return -1;
        }

        free(sigilData);
        startPos += chunk;
    }
    printString(1, "Sigil sent successfully.\n");
    free(filename);
    close(fdSigil);

    return 1;
}

ForkRegneWritesEnvoy *crearForkRegneWritesEnvoy(int comanda,const char *regneDest,const char *rutaSegell,int indexEnvoy,int ackStatus,int statusPledgeRespond,const char *ipDest,unsigned int portDest) {
    ForkRegneWritesEnvoy *frwe = malloc(sizeof(ForkRegneWritesEnvoy));
    
    if (frwe == NULL) {
        printString(1, "Error allocating memory for ForkRegneWritesEnvoy\n");
        return NULL;
    }
    memset(frwe, 0, sizeof(ForkRegneWritesEnvoy));

    frwe->comanda = comanda;
    frwe->indexEnvoy = indexEnvoy;
    frwe->ackStatus = ackStatus;
    frwe->statusPledgeRespond = statusPledgeRespond;
    frwe->portDest = portDest;

    if (regneDest != NULL) {
        strncpy(frwe->regneDest, regneDest, sizeof(frwe->regneDest) - 1);
        frwe->regneDest[sizeof(frwe->regneDest) - 1] = '\0';
    } else {
        frwe->regneDest[0] = '\0';
    }

    if (rutaSegell != NULL) {
        strncpy(frwe->rutaSegell, rutaSegell, sizeof(frwe->rutaSegell) - 1);
        frwe->rutaSegell[sizeof(frwe->rutaSegell) - 1] = '\0';
    } else {
        frwe->rutaSegell[0] = '\0';
    }

    if (ipDest != NULL) {
        strncpy(frwe->ipDest, ipDest, sizeof(frwe->ipDest) - 1);
        frwe->ipDest[sizeof(frwe->ipDest) - 1] = '\0';
    } else {
        frwe->ipDest[0] = '\0';
    }

    return frwe;
}


int trobarEnvoyIndex(Regne *r, const char *regneDest) {
    int indexEnvoy = -1;
    for (int i = 0; i < r->n_envoys; i++)
    {

        if (strcmp(r->envoys[i].nomRegneViatge, regneDest) == 0) {
            indexEnvoy = i;
            break;
        }
    }
    return indexEnvoy; 
}
int trobarRutaIndex(Regne *r, const char *regneDest) {
    int indexRuta = -1;
    for (int i = 0; i < r->n_rutes; i++)
    {
        if (strcmp(r->rutes[i].nomDest, regneDest) == 0) {
            indexRuta = i;
            break;
        }
    }
    return indexRuta;
}
int trobarEnvoyLliure(Regne *r) {
    for (int i = 0; i < r->n_envoys; i++) {
        if (r->envoys[i].status == 0) {//si esta lliure  el envoy ...
            return i;
        }
    }
    return -1; 
}

int setEnvoyEnMissio(Regne *r, int indexEnvoy, const char *regneDest) {
    if (indexEnvoy < 0 || indexEnvoy >= r->n_envoys || regneDest == NULL) {
        printString(1, "Invalid envoy index\n");
        return -1;
    }
    r->envoys[indexEnvoy].status = 1; // 1 = ON MISSION
    r->envoys[indexEnvoy].nomRegneViatge = malloc(strlen(regneDest) + 1);
    if (r->envoys[indexEnvoy].nomRegneViatge == NULL) {
        printString(1, "Error allocating memory for envoy mission\n");
        r->envoys[indexEnvoy].status = 0;
        return -1;
    }
    strcpy(r->envoys[indexEnvoy].nomRegneViatge, regneDest);
    return 0;
}
int setEnvoyLliure (Regne *r, int indexEnvoy) {
    if (indexEnvoy < 0 || indexEnvoy >= r->n_envoys) {
        printString(1, "Invalid envoy index\n");
        return -1;
    }
    r->envoys[indexEnvoy].status = 0; // 0 = lliure
    if (r->envoys[indexEnvoy].nomRegneViatge != NULL) {
        free(r->envoys[indexEnvoy].nomRegneViatge);
        r->envoys[indexEnvoy].nomRegneViatge = NULL;
    }
    return 0;
}



