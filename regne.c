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
@brief Funció que gestiona l'enviament d'un pledge a un altre regne.
@param arg Punter a la estructura Thread_pledge que conté la informació necessària per
               processar el pledge.

*/
void* pledge(void *arg){
    Thread_pledge *tp = (Thread_pledge *) arg;
    Regne *r = tp->r;
    char **paraules = tp->paraules;
    //comprovar que el regne existeix
    //printf("r->nom: %s == paraules[1]: %s\n", r->nom, paraules[1]);
    if (!comprovarRegneExisteix(r, paraules[1])) {
        printString(1, "No such realm exists. The pledge is hereby withdrawn. Remember is case sensitive\n");
        freeThreadPledge(tp);
        return NULL;
    }

    //obrir fitxer segell per comprovar que existeix
    /*char *filename = malloc(FILENAME_SIGIL_SIZE * sizeof(char));
    if (!filename) {
        freeThreadPledge(tp);
        return NULL;
    }*/
    //snprintf(filename, FILENAME_SIGIL_SIZE, "segells/%s", paraules[2]);
    char *filename;
    asprintf(&filename, "segells/%s", paraules[2]);
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        printString(1, "Sigil doesn't exist\n");
        free(filename);
        freeThreadPledge(tp);
        return NULL;
    }
    
    //comprovar si ja tenim una aliança o pledge pendent
    int indexRuta = -1;
    for (int i = 0; i < r->n_rutes; i++) {
        if (strcmp(r->rutes[i].nomDest, paraules[1]) == 0) {
            if (r->rutes[i].alianca == 'S') {
                printString(1, "You are already allied with this Realm.\n");
                close(fd);
                free(filename);
                freeThreadPledge(tp);
                return NULL;
            }
            if (r->rutes[i].alianca == 'P') {
                printString(1, "You have already sent an alliance request to this Realm.\n");
                close(fd);
                free(filename);
                freeThreadPledge(tp);
                return NULL;
            }
            if (r->rutes[i].alianca == 'O') {
                printString(1, "You have a pending pledge from that Maester.\n");
                close(fd);
                free(filename);
                freeThreadPledge(tp);
                return NULL;
            }

            
            indexRuta = i;
            break;
        }
    }
    if (indexRuta == -1) {
        printString(1, "Route not found for this Realm.\n");
        close(fd);
        free(filename);
        freeThreadPledge(tp);
        return NULL;
    }
    //crear el directori si no esta creat 
    crearDirectori(r->rutaArxius);
    //reiniciem l'envoy per si de cas
    resetEnvoy(&r->envoys[tp->indexEnvoy]);
    //Aqui assignem l'envoy que ara esta fent un pledge
    setStatusEnvoy(&r->envoys[tp->indexEnvoy], "pledge");
    r->rutes[indexRuta].alianca = 'P';
    
    //calcular MD5 i size per posar a la trama 0x001
    char data[DATA_SIZE];
    int size = getSize(fd);
    char *md5 = calcularMD5SUM(filename);
    if (!md5) {
        printString(1, "Error calculant MD5\n");
        close(fd);
        free(filename);
        resetEnvoy(&r->envoys[tp->indexEnvoy]);
        freeThreadPledge(tp);
        return NULL;
    }
    snprintf(data, DATA_SIZE, "%s&%s&%d&%s", r->nom, paraules[2], size, md5);
    
    //enviar la trama 0x01 per hops
    Trama *trama = crearTrama(0x01, r->ip, r->port, paraules[1], strlen(data), data);
    int socket = enviarTramaDinamica(trama, r, 0); // trama s'allibera aquí
    if (socket < 0) {
        printString(1, "Cannot send pledge request.\n");
        resetEnvoy(&r->envoys[tp->indexEnvoy]);
        r->rutes[indexRuta].alianca = 'N'; //N de none
        freeThreadPledge(tp);
        return NULL;
    }

    close(fd);
    free(filename);
    free(md5);
    if(r->envoys[0].ackFitxer==2){//si hi ha hagut un error enviant la trama 0x01
        printString(1, "Error sending pledge request. Cannot conect to realm.");
        r->rutes[indexRuta].alianca='N'; //N de none
        resetEnvoy(&r->envoys[tp->indexEnvoy]);
        freeThreadPledge(tp);
        return NULL;
    }
     
    char *buffer = NULL;
    if (asprintf(&buffer, "Pledge sent to %s. Envoy 1 is on its way.\n", paraules[1]) != -1) {
        printString(1, buffer);
        free(buffer);
    }

    int startPos = 0;
    int fdSigil = obrirFitxerSigil(paraules[2]);
    if (fdSigil == -1) {
        printString(1, "Error opening sigil file\n");
        r->rutes[indexRuta].alianca = 'N';
        resetEnvoy(&r->envoys[tp->indexEnvoy]);
        freeThreadPledge(tp);
        return NULL;
    }

    usleep(100000);//espera per si de cas per que arribi la trama anterior correctament (ja que si no no fa un reset del fitxer)
    //Començem a enviar el segell
    struct timeval startTime, endTime;
    gettimeofday(&startTime, NULL);
    printString(1, "Sending sigil ...\n");
    while (startPos <= size) {
      
        usleep(10000);
        //usleep(1000);

        if(r->envoys[0].ackFitxer==2){//si hi ha hagut un error enviant la trama 0x02
            printString(1, "Error sending sigil data. Cannot conect to realm.");
            r->rutes[indexRuta].alianca='N'; //N de none
            close(fdSigil);
            resetEnvoy(&r->envoys[tp->indexEnvoy]);
            freeThreadPledge(tp);
            
            return NULL;
        }
        if (r->exit && *(r->exit)) {//per si fa control C
            close(fdSigil);
            resetEnvoy(&r->envoys[tp->indexEnvoy]);
            freeThreadPledge(tp);
            return NULL;
        }
        int chunk = (startPos + 275 > size) ? (size - startPos) : 275;//si supera el size ajustem el chunk ja que la ultima trama  no sera 275
        if (chunk <= 0) break;
        char *sigilData = readFd(fdSigil, startPos, chunk);//llegim el fitxer del segell marcat per el if anterior(chunk pot ser mes petit que 275 o 275 maxim)
        if (!sigilData) {
            printString(1, "Error reading sigil data\n");
            close(fdSigil);
            r->rutes[indexRuta].alianca = 'N';
            resetEnvoy(&r->envoys[tp->indexEnvoy]);
            freeThreadPledge(tp);
            return NULL;
        }
        //crear i enviar la trama 0x02 per hops , es la trama per enviar el segell trama a trama
        trama = crearTrama(0x02, r->ip, r->port, r->rutes[indexRuta].nomDest, chunk, sigilData);
        int socket = enviarTramaDinamica(trama, r, 0);

        if (socket < 0) {
            printString(1, "Error sending sigil trama.\n");
            free(sigilData);
            close(fdSigil);
            r->rutes[indexRuta].alianca = 'N';
            resetEnvoy(&r->envoys[tp->indexEnvoy]);
            freeThreadPledge(tp);
            return NULL;
        }

        free(sigilData);
        startPos += chunk;
        joinThreadServidor(r);
    }
  gettimeofday(&endTime, NULL);
    long tempsMs = (endTime.tv_sec - startTime.tv_sec) * 1000 + (endTime.tv_usec - startTime.tv_usec) / 1000;
    char *misstemps;
    asprintf(&misstemps, "Sigil sent. (%ld ms)\n", tempsMs);
    printString(1, misstemps);
    free(misstemps);
    close(fdSigil);
    
    //aqui vol dir que les acks estan be(ackFitxer i md5sum), per tant toca esperar el timeout de 2min per que faigi el pledge respond 
    //el servidor rebre el pledge respond i modificara 
    time_t inici = time(NULL);
    const double timeout = 120.0;

    while (1) {
        sleep(1);
        if (r->exit && *(r->exit)) break;
        if (r->rutes[indexRuta].alianca == 'S' || r->rutes[indexRuta].alianca == 'N') {//si ja ha rebut el pledge respond haruia de ser S o N, sino encara esta pendent
            break;
        }
        if (r->envoys[0].ackFitxer==2 || r->envoys[0].ackMD5SUM==2) {
            r->rutes[indexRuta].alianca = 'N';
            break;
        }
        if (difftime(time(NULL), inici) >= timeout) {
            char *tmp = NULL;
            if (asprintf(&tmp, "Pledge to %s timed out.\n", r->rutes[indexRuta].nomDest) != -1) {
                printString(1, tmp);
                free(tmp);
            }
            r->rutes[indexRuta].alianca = 'T'; // T de timeout
            break;
        }
    }
    //envoy acaba pledge
    
    resetEnvoy(&r->envoys[tp->indexEnvoy]);
    freeThreadPledge(tp);


    return NULL;
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
            Trama *t_enviar = crearTrama(0x03, r->ip, r->port,r->rutes[trobat].nomDest, strlen(data), data);
            int socket = enviarTramaDirecteSenseTancar(t_enviar,r->rutes[trobat].portDest,r->rutes[trobat].ipDest,paraules[2]);
            /*if (socket < 0) {
                return;
            }*/
            //esperar ack/nack
            Trama *t = malloc(sizeof(Trama));
            /*if (!t) {
                close(socket);
                return;
            }*/
            /*int n1 = read(socket, t, sizeof(Trama));
            if (n1 <= 0) {
                free(t);
                printString(1, "Did not receive ACK/NACK for pledge response.\n");
                close(socket);
                return;
            }*/
            //if (t->tipus == 0x31) { // ACK correcte
                char *miss;
                asprintf(&miss, "Alliance with %s established.\n", paraules[2]);
                printString(1, miss);
                free(miss);

                r->rutes[trobat].alianca = 'S';
                return;
            /*}
            else if (t->tipus == 0x69) { // NACK
                char *miss2;
                asprintf(&miss2, "Alliance with %s not established.\n", paraules[2]);
                printString(1, miss2);
                free(miss2);
            }
            else {
                char *miss3;
                asprintf(&miss3, "Did not receive valid ack from %s\n", paraules[2]);
                printString(1, miss3);
                free(miss3);
            }

            close(socket);
            free(t);
            return;
        }

        else {
            printString(1, "Please respond with accept or reject.\n");
            return;
        }*/
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

void envoyStatus (__attribute__((unused)) Regne *r){

    printString(1, "Command OK\n");
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
void readEnvoys(fd_set *fds_set, Regne *r) {
    for (int i = 0; i < r->n_envoys; i++) {
        int fd = r->envoys[i].pipeRead;
        if (FD_ISSET(fd, fds_set)) {
            EnvoyWritesRegne msg;
           // char buffer[275];
            int n = read(fd, &msg, sizeof(msg));
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
void processarMissatgeEnvoy(Regne *r, EnvoyWritesRegne *msg) {
    Envoy *e = &r->envoys[msg->indexEnvoy];

    printf("Processing message from Envoy %d: Command %d\n", msg->indexEnvoy, msg->comanda);
    switch (msg->comanda) {
        case 1://pledge
        printf("msg->comanda -> %d\n", msg->comanda);
            if(msg->statusPledge==1){//accpet

                r->rutes[msg->indexRuta].alianca = 'S'; 
            }else if(msg->statusPledge==2){ //reject

                r->rutes[msg->indexRuta].alianca = 'N'; 

            }else{//timeout
            r->rutes[msg->indexRuta].alianca = 'T'; 

            }
            
            break;

        case 2://list products
            break;

        case 3://send 
            break;
    }
}














int pledgeComprovar(Regne *r, char **paraules, int n_paraules){

    if (!comprovarRegneExisteix(r, paraules[1])) {
        printString(1, "No such realm exists. The pledge is hereby withdrawn. Remember is case sensitive\n");
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
void pledgeEnvoyTramaInicial(char filenameSigil[30] , char nomDesti[50], Regne *r, int indexRuta){

   char *filename;
    asprintf(&filename, "segells/%s", filenameSigil);
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        printString(1, "Sigil doesn't exist\n");
        free(filename);
        return NULL;
    }

    char data[DATA_SIZE];
    int size = getSize(fd);
    char *md5 = calcularMD5SUM(filename);
    if (!md5) {
        printString(1, "Error calculant MD5\n");
        close(fd);
        free(filename);

        return NULL;
    }
    snprintf(data, DATA_SIZE, "%s&%s&%d&%s", r->nom, filename, size, md5);

 

     Trama *trama = crearTrama(0x01, r->ip, r->port,nomDesti , strlen(data), data);
    int socket = enviarTramaDinamica(trama, r, 0); // trama s'allibera aquí
    if (socket < 0) {
        printString(1, "Cannot send pledge request.\n");
        //r->rutes[indexRuta].alianca = 'N'; //N de none
        return NULL;
    }

    close(fd);
    free(md5);


    char *buffer = NULL;
    if (asprintf(&buffer, "Pledge sent to %s. Envoy 1 is on its way.\n", nomDesti) != -1) {
        printString(1, buffer);
        free(buffer);
    }
    return ;
}
void pledgeEnvoyEnviarSigil(char filenameSigil[30] , char nomDesti[50], Regne *r, int indexRuta,int *exitflag){

    int startPos = 0;
    char *filename;
    asprintf(&filename, "segells/%s", filenameSigil);
    int fdSigil = open(filename, O_RDONLY);
    if (fdSigil == -1) {
        printString(1, "Sigil doesn't exist\n");
        free(filename);
        return NULL;
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
            return NULL;
        }
        int chunk = (startPos + 275 > size) ? (size - startPos) : 275;//si supera el size ajustem el chunk ja que la ultima trama  no sera 275
        if (chunk <= 0) break;
        char *sigilData = readFd(fdSigil, startPos, chunk);//llegim el fitxer del segell marcat per el if anterior(chunk pot ser mes petit que 275 o 275 maxim)
        if (!sigilData) {
            printString(1, "Error reading sigil data\n");
            close(fdSigil);
            //r->rutes[indexRuta].alianca = 'N';
            return NULL;
        }
        //crear i enviar la trama 0x02 per hops , es la trama per enviar el segell trama a trama
        trama = crearTrama(0x02, r->ip, r->port,nomDesti , chunk, sigilData);
        int socket = enviarTramaDinamica(trama, r, 0);

        if (socket < 0) {
            printString(1, "Error sending sigil trama.\n");
            free(sigilData);
            close(fdSigil);
            //r->rutes[indexRuta].alianca = 'N';
            return NULL;
        }

        free(sigilData);
        startPos += chunk;
    }
    free(filename);
    return NULL;
}

int pledgeEnvoyWaitPledgeRespond(Regne *r, char nomDest[50], int *exitflag,int *pledgeRespondRebut){
    time_t inici = time(NULL);
    const double timeout = 120.0;
    //return -1 si fa control c, 0 si fa timeout, 1 si ha rebut pledge respond
    while (1) {
        sleep(1);
        //surtira daqui si fa control C o si ja ha rebut el pledge respond
        if (*exitflag){
            return -1;
        }
        if(*pledgeRespondRebut) {
            return 1;
        } 
        if (difftime(time(NULL), inici) >= timeout) {
            char *tmp = NULL;
            if (asprintf(&tmp, "Pledge to %s timed out.\n", nomDest) != -1) {
                printString(1, tmp);
                free(tmp);
            }
            //timeout
           // r->rutes[indexRuta].alianca = 'T'; // T de timeout
           return 0;
        }
    }
    return 0;
}
    
/*
    


    //espera per si de cas per que arribi la trama anterior correctament (ja que si no no fa un reset del fitxer)
    //Començem a enviar el segell
    struct timeval startTime, endTime;
    gettimeofday(&startTime, NULL);
    
    
    
    gettimeofday(&endTime, NULL);
    long tempsMs = (endTime.tv_sec - startTime.tv_sec) * 1000 + (endTime.tv_usec - startTime.tv_usec) / 1000;
    char *misstemps;
    asprintf(&misstemps, "Sigil sent. (%ld ms)\n", tempsMs);
    printString(1, misstemps);
    free(misstemps);
    close(fdSigil);
    
    //aqui vol dir que les acks estan be(ackFitxer i md5sum), per tant toca esperar el timeout de 2min per que faigi el pledge respond 
    //el servidor rebre el pledge respond i modificara 
    time_t inici = time(NULL);
    const double timeout = 120.0;

    while (1) {
        sleep(1);
        if (r->exit && *(r->exit)) break;
        if (r->rutes[indexRuta].alianca == 'S' || r->rutes[indexRuta].alianca == 'N') {//si ja ha rebut el pledge respond haruia de ser S o N, sino encara esta pendent
            break;
        }
        if (r->envoys[0].ackFitxer==2 || r->envoys[0].ackMD5SUM==2) {
            r->rutes[indexRuta].alianca = 'N';
            break;
        }
        if (difftime(time(NULL), inici) >= timeout) {
            char *tmp = NULL;
            if (asprintf(&tmp, "Pledge to %s timed out.\n", r->rutes[indexRuta].nomDest) != -1) {
                printString(1, tmp);
                free(tmp);
            }
            //TODO: aqui hem de comunicar que ha fet timeout
           // r->rutes[indexRuta].alianca = 'T'; // T de timeout
            break;
        }
    }*/
    


/*void* pledge(void *arg){
   

    //TODO: aixo ho fara el maester quan fagi el write cap el envoy
    r->rutes[indexRuta].alianca = 'P';
    
    //calcular MD5 i size per posar a la trama 0x001
    
    
    //enviar la trama 0x01 per hops
   

    return NULL;
}*/
