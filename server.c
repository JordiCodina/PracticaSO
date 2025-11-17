

#include "server.h"


#define ERROR_MALLOC "Error: malloc falló\n"
#define ERROR_REALLOC "Error: realloc falló\n"
#define WAITING_CONNECTION "Esperando conexiones...\n"
#define CLIENT_CONNECTED "¡Cliente conectado!\n"
#define USAGE "Uso: ./server <palabra> <max_intentos> <puerto>\n"
#define ERROR_SOCKET "Error en la creación del socket"
#define ERROR_BIND "Error en el bind del socket\n"
#define ERROR_LISTEN "Error en el listen del socket\n"
#define ERROR_ACCEPT "Error en el accept de la conexión\n"
#define MAX_TRIES "Error: max_intentos debe ser mayor que 0 y al menos 3 más que la longitud de la palabra\n"
#define BUFFER_SIZE 256
 
int create_server_socket(int port) {
    int server_fd;
    struct sockaddr_in address;
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        printString(1,ERROR_SOCKET);
        return -1;
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        printString(1,ERROR_BIND);
        close(server_fd);
        return -1;
    }
    
    if (listen(server_fd, 3) < 0) {
        printString(1,ERROR_LISTEN);
        close(server_fd);
        return -1;
    }
    
    return server_fd;
}
int accept_client_connection(int server_fd) {
    int new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t*) &addrlen);
    if (new_socket < 0) {
        printString(1,ERROR_ACCEPT);
        return -1;
    }
    
    return new_socket;
}

void printTrama(Trama t){//aixo es per debugar pero es borrara
        printf("─────────────\n");
        printf("Tipus: 0x%02X\n", t.tipus);
        printf("Origen(ip+port): %s\n", t.nom_origen);
        printf("NomDestí: %s\n", t.nom_desti);
        printf("Longitud dades: %d\n", t.dataLength);
        printf("Dades: %s\n", t.data);
        printf("Checksum: %u (OK)\n", t.checksum);
        printf("─────────────\n");
} 


/*

ENVAIMENT TRAMES ESPECÍFIQUES

*/

void enviarTramaACKFitxer(char * ip, int port, char *nomDest,char * status ){//status es OK o KO
    printf("Enviant ACK de fitxer a %s:%d per %s amb estat %s\n", ip, port, nomDest, status);
    Trama * tramaNACK=crearTrama(0x31, NULL, 0, NULL,2, status);
    enviarTramaDirecte(tramaNACK, port, ip, nomDest);
    free(tramaNACK);
}
void enviarTramaNACK(char * ip, int port, char *nomDest ){
    Trama * tramaNACK=crearTrama(0x69, NULL, 0, NULL,0, NULL);
    enviarTramaDirecte(tramaNACK, port, ip, nomDest);
    free(tramaNACK);
}
/*void enviarTramaMD5SUM(Regne *r, unsigned short llargariaDades){

    Trama * tramaMD5SUM=crearTrama(0x32, NULL, 0, NULL, llargariaDades, "CHECK_OK");
    enviarTramaDirecte(tramaMD5SUM, r->port, r->ip, "MDSUM5");
    free(tramaMD5SUM);
}*/


/*

RESPONDRE A PETCIONS DEL CLIENT COM A SERVIDOR

*/
int response=-1; //varaible global per guardar la resposta del pledge (encara nse si es fa aixi)

void  respondPledge(int res){//1 acceptat 0 denegat 
    response=res;
}



void* esperarPledgeRespond(void *conf_ptr){
    ClientConf *clientConf = (ClientConf *)conf_ptr;
    printf("Waiting for pledge response...\n");
    time_t inici = time(NULL);          // hora d’inici
    const double timeout = 120.0;
    while(response == -1){ 
        sleep(1);//1 segon d'espera
        time_t ara = time(NULL);
        double diff = difftime(ara, inici);
        if (diff >= timeout) {          
            response = 2;              
            printf("Timeout de 2 minuts.\n");
            break;
        }
        
    }
    if(response == 1){
        strncpy(clientConf->trama->data, "ACCEPT", sizeof(clientConf->trama->data)-1);
    } else if (response == 2) {
        printString(1, "Pledge response timed out.\n");
        return NULL;
    }else{
        strncpy(clientConf->trama->data, "REJECT", sizeof(clientConf->trama->data)-1);
    }
    enviarTramaDirecte(clientConf->trama, clientConf->portDest, clientConf->ipDest, clientConf->nomDest);
    response=-1;
    free(clientConf);
    return NULL;
}


 void processarTramaPledge(Trama *t, Regne *r) {
    int n_paraules = 0;
    int n_paraules2 = 0;
    char **paraules  = tokenizer(t->data, &n_paraules, '&');
    if (n_paraules != 4) {
        freeTokens(paraules, n_paraules);
        return;
    }
    for (int i = 0; i < r->n_rutes; i++) {
        if (strcmp(r->rutes[i].nomDest, paraules[0]) == 0) {
            r->rutes[i].alianca = 'P';
            break;
        }
    }
    char **paraules2 = tokenizer(t->nom_origen, &n_paraules2, ':');//ip:port
    Trama *novatrama = crearTrama(0x03, r->ip, r->port,paraules[0], strlen("ACCEPT/REJECT"), "ACCEPT/REJECT");
    ClientConf *conf = trobarSeguentMaester(r, novatrama);
    conf->trama = novatrama;
    pthread_t pledgerespondthread;
    pthread_create(&pledgerespondthread, NULL,esperarPledgeRespond, (void *)conf);//aqeust thread crec que no serveix per molt la veritat
    pthread_join(pledgerespondthread, NULL);
    freeTokens(paraules, n_paraules);
    freeTokens(paraules2, n_paraules2);
}

 void processarTramaPledgeRespond(Trama *t, Regne *r) {
    
    int n_paraules = 0;
    char **paraules = tokenizer(t->nom_origen, &n_paraules, ':');
    if (n_paraules != 2) {
        freeTokens(paraules, n_paraules);
        printString(1, "Pledge respond trama malformed\n");
        return;
    }

   
     
    if (strcasecmp(t->data, "ACCEPT") == 0) {
        for (int i = 0; i < r->n_rutes; i++) {
            printf("Comparant %s amb %s\n", r->rutes[i].nomDest, t->nom_desti);
            if (strcmp(r->rutes[i].nomDest, t->nom_desti) == 0) {
                printf("Alliance accepted for realm %s\n", r->rutes[i].nomDest);
                r->rutes[i].alianca = 'S';

                if (r->rutes[i].esConegut == 0) {
                    r->rutes[i].portDest = atoi(paraules[1]);
                    r->rutes[i].ipDest   = paraules[0];
                }
                printString(1, "Alliance accepted.\n");
                break;
            }
        }
    } else if (strcasecmp(paraules[0], "REJECT") == 0) {
        for (int i = 0; i < r->n_rutes; i++) {
            if (strcmp(r->rutes[i].nomDest, t->nom_origen) == 0) {
                r->rutes[i].alianca = 'N';
                printString(1, "Alliance rejected.\n");
                break;
            }
        }
    }
    //freeTokens(paraules, n_paraules);
}

/*

AQUI ES PROCESSEN TOTES LES TRAMES A PARTIR DEL TIPUS

*/
 void processarTrama(Trama *t, Regne *r) {

    switch(t->tipus){

        case 0x01: 
                int n_paraules = 0;
                char **paraules = tokenizer(t->nom_origen, &n_paraules, ':');
                enviarTramaACKFitxer(paraules[0], atoi(paraules[1]), t->nom_desti, "OK");
                freeTokens(paraules, n_paraules);
              
                processarTramaPledge(t, r);
                break;
        
            case 0x02:
                printString(1, "Trama segell  rebuda:\n");
             
                break;
            case 0x03:
                processarTramaPledgeRespond(t, r);
                
                break;
            case 0x11: 
                printString(1, "Trama llistar productes rebuda:\n");
                
                break;
            case 0x12:
                printString(1, "Trama resposta llistar productes rebuda:\n");
               
                break;
            case 0x13:
                printString(1, "Trama envia dades producte rebuda:\n");
                
                break;
            case 0x14:
                printString(1, "Trama shoping list header rebuda:\n");
               
                break;
            case 0x15:

                printString(1, "Trama enviament de fitxer productes rebuda:\n");
              
                break;
            case 0x16:
                printString(1, "Trama  rebuda:\n");
              
                break;
            case 0x27:
                printString(1, "Trama desconexió rebuda:\n");
               
                break;
            case 0x21:
                printString(1, "Trama regne desconegut rebuda:\n");
              
                break;
            case 0x25:
                printString(1, "Trama no aliança per productes rebuda:\n");
              
                break;
            case 0x26:
                printString(1, "Trama ping rebuda:\n");
                
                break;
            case 0x31:
                /*if(strcmp(t->data,"OK")){
                printString(1, "Trama ack(OK) rebuda:\n");
                }else{
                    printString(1, "Trama ack(KO) rebuda:\n");
                }*/
                
                break;
            case 0x32:
                printString(1, "Trama md5sum rebuda:\n");
             
                break;
            case 0x69:
                printString(1, "Trama error : NACK  rebuda:\n");
                
                break;           
            default:
                printString(1, "Tipus de trama desconegut.\n");
                break;
        }
    }


int tocaIgnorarTrama(int tipus){
    if(tipus==0x69 || tipus==0x31 || tipus==0x32){//poden haver-hi mes
        return 1;
    }
    return 0;
}
int comprovarChecksum(Trama *t){
    unsigned short checksumCalculat = calcularChecksum(t);
    if (checksumCalculat != t->checksum) {
        return 0;
    }
    return 1;
}

void* runServer(void *aregne) {
    Regne *r = (Regne *)aregne;
    int server_fd = create_server_socket(r->port);
    if (server_fd < 0) {
        printString(1, "Error creant el servidor\n");
        return (void*)1;
    }
    while (1) {
        //printString(1, "Maester: Esperant connexions...\n");
        int new_socket = accept_client_connection(server_fd);
        if (new_socket < 0) {
            printString(1, "Error acceptant connexió\n");
            continue;
        }
        
        Trama* t = crearTrama(0, "", 0, "", 0, ""); //inicialitzar trama
        ssize_t servRead = read(new_socket, t, sizeof(Trama));
            if (servRead <= 0) {
                printString(1, "Connexió tancada o error.\n");
                close(new_socket);
                break;
            }
            if (servRead != sizeof(Trama)) {
                printString(1, "Trama rebuda incompleta.\n");
                close(new_socket);
                continue;
            }
            
            if(tocaIgnorarTrama(t->tipus)){//ignorar trames de tipus ACK,NACK,MD5SUM i tal , que les processi directament sense passar perl passos seguents,sense fer checksum ni que les reenvii
                processarTrama(t, r);
                close (new_socket);
                free(t);
                continue;
            }

           /* if (!comprovarChecksum(t) && t->checksum>0) {//compovem checksum
                    int n_paraules = 0;
                    char **paraules = tokenizer(t->nom_origen, &n_paraules, ':');
                    //enviarTramaACKFitxer(paraules[0], atoi(paraules[1]), t->nom_desti, "KO");
                    enviarTramaNACK(paraules[0], atoi(paraules[1]), t->nom_desti);
                    freeTokens(paraules, n_paraules);
                    close(new_socket);
                    free(t);
                    continue;
            }*/

            //aqui haurem de comprovar el MD5SUM 0x32


            if (strcmp(t->nom_desti, r->nom) == 0) {//aqui es el desti final
                processarTrama(t, r);
            }else {//aqui fa hopping
                enviarTrama(t, r, 1);
            }
            close(new_socket);

            free(t);
    }
    close(server_fd);
    printString(1, "Maester: Socket tancat\n");
    return NULL;
}
