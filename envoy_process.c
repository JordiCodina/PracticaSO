
#include "envoy_process.h"


int exitEnvoyFlag = 0;

void signalHandlerEnvoy() {
    exitEnvoyFlag = 1;
}

int mirarTimeoutPledge(int mirarTimeoutFlag,double timeout, time_t inici) {
        if (mirarTimeoutFlag <= 0) return 0;

        if (difftime(time(NULL), inici) >= timeout) {
            return 2;
    }
    return 1;
} 


void notificarRegneErrorMissio(int comanda,int pipeWrite, ForkRegneWritesEnvoy infoInicial, Regne *r) {//comanda 4 = error missio( no es timeout ni accept ni reject)
    //errors pot ser per acks , errors de connexio , etc
    EnvoyWritesRegne responseTimeout;
    memset(&responseTimeout, 0, sizeof(EnvoyWritesRegne));
    responseTimeout.comanda = comanda;
    responseTimeout.indexEnvoy = infoInicial.indexEnvoy;
    int indexRuta=trobarRutaIndex(r, infoInicial.regneDest);
    responseTimeout.indexRuta=indexRuta;//no es fa servir
    responseTimeout.statusPledge = 0; //no es fa servir
    write(pipeWrite, &responseTimeout, sizeof(EnvoyWritesRegne));

}


void runenvoy(int pipeRead, int pipeWrite, Regne *r) {

    int statusACKFitxer=0;
    int statusACKMD5=0;
    time_t inici;
    const double timeoutPledge = 120.0;
    int mirarTimeout=0;
    int indexRuta=-1;
    signal(SIGINT, signalHandlerEnvoy);

    fd_set read_set;
    struct timeval timeout;
    ForkRegneWritesEnvoy infoInicial;

    EnvoyWritesRegne response;
    response.portDest = r->port;
    strcpy(response.ipDest, r->ip);
    //printf("\tmsg port: %u, msg ip: (%s)\n", response.portDest, response.ipDest);

    while (1) {
        if (exitEnvoyFlag) break;

        FD_ZERO(&read_set);
        FD_SET(pipeRead, &read_set);

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int ret = select(pipeRead + 1, &read_set, NULL, NULL, &timeout);

        if (ret < 0) continue;
        if(ret == 0) {
             int statusTimeout = mirarTimeoutPledge(mirarTimeout,timeoutPledge, inici);
             if(statusTimeout==2) {//indica que ha fet timeout
                    mirarTimeout=0;
                    response.comanda = 1;
                    response.indexEnvoy = infoInicial.indexEnvoy;
                    int indexRuta=trobarRutaIndex(r, infoInicial.regneDest);
                    response.indexRuta=indexRuta;
                    response.statusPledge = 0; // write TIMEOUT

                    write(pipeWrite, &response, sizeof(EnvoyWritesRegne));
             }
            continue;
        } 


        if (FD_ISSET(pipeRead, &read_set)) {
            ForkRegneWritesEnvoy buffer;
            int n = read(pipeRead, &buffer, sizeof(ForkRegneWritesEnvoy));
            if (n <= 0) break;

          
           
            switch (buffer.comanda) {
                case 0: // PLEDGE
                    //   
                    mirarTimeout=0;
                    
                    infoInicial=buffer;
                    int status = pledgeEnvoyTramaInicial(infoInicial.rutaSegell, infoInicial.regneDest,r);
                    if(!status){
                        notificarRegneErrorMissio(4, pipeWrite, infoInicial, r);
                    }

                    break;
                case 1: // LIST PRODUCTS
                    indexRuta=trobarRutaIndex(r, buffer.regneDest); 
                    if(indexRuta<0){ 
                        return;
                    }
                    listProductsAliat(r, buffer.regneDest,indexRuta);
                    break;
                case 2: // SEND PRODUCTS
                    indexRuta=trobarRutaIndex(r, buffer.regneDest);
                    if(indexRuta<0){ 
                        return; //cas imposible
                    }
                    int statusSendTrade = sendTrade( crearRutaComanda( r->rutaArxius, r->nom, buffer.regneDest), buffer.regneDest, r,indexRuta);
                    if(!statusSendTrade){
                        notificarRegneErrorMissio(4, pipeWrite, infoInicial, r);
                    }

                    break;
                case 3: // (PLEDGE) REBRE ACK FITXER -> si OK -> ENVIAR SIGIL
                    statusACKFitxer=buffer.ackStatus; // 1 = OK , 2 = KO
                    int statusSigil = -1;

                    if(statusACKFitxer==1){
                        statusSigil =pledgeEnvoyEnviarSigil( infoInicial.rutaSegell, infoInicial.regneDest, r, &exitEnvoyFlag);
                    }
                    if (statusACKFitxer != 1 || statusSigil == -1) {
                        
                        mirarTimeout=0;
                        notificarRegneErrorMissio(4, pipeWrite, infoInicial, r);
                    }
                break;
                case 4:// (PLEDGE) REBRE ACK MD5SUM -> OK -> COMENÇAR 2 minuts

                    statusACKMD5=buffer.ackStatus; // 1 = OK , 2 = KO
                    if(statusACKMD5==1){
                        mirarTimeout=1;
                        inici= time(NULL); //guardem temps inici per mirar timeout                        
                    }else{
                        mirarTimeout=0;
                        notificarRegneErrorMissio(4, pipeWrite, infoInicial, r);
                    }
                    break;
                case 5:// (PLEDGE respond)rebre pledge respond
                    
                    
                    response.comanda = 1;
                    response.indexEnvoy = infoInicial.indexEnvoy;
                    int indexRuta=trobarRutaIndex(r, infoInicial.regneDest);
                    response.indexRuta=indexRuta;
                    if(indexRuta==-1){
                        printString(1, "No route found for the specified realm in pledge respond.\n");
                        break;
                    }
                    if(mirarTimeoutPledge(mirarTimeout,timeoutPledge, inici)) {
                         mirarTimeout=0;
                        if(buffer.statusPledgeRespond==1) // 1 = ACCEPT , 2 = REJECT , 0 = TIMEOUT
                        { 
                            response.statusPledge = 1; // write  ACCEPT

                            //ARA MATEIX SESTA ENVIANT EL IP I PORT DEL TEU REGE
                            //S'HAURIEN DENVIAR EL IP I PORT DEL REGNE AL QUAL HAS FET EL PLEDGE
                            //S'HA DE CANVIAR
                            response.portDest = r->port;
                            strcpy(response.ipDest, r->ip);

                            write(pipeWrite, &response, sizeof(EnvoyWritesRegne));
                        }else{
                            response.statusPledge = 2;
                            write(pipeWrite, &response, sizeof(EnvoyWritesRegne)); 
                            //write reject
                        }
                    }else{ //timeout
                        response.statusPledge = 0;
                        write(pipeWrite, &response, sizeof(EnvoyWritesRegne));
                       

                    }
                     mirarTimeout=0;                  
                    
                break;

                break;
                default:
                    printString(1, "Unknown order received by envoy.\n");
                    break;
            }

        }
    }

    printf("Envoy PID %d exiting...\n", getpid());
    close(pipeRead);
    close(pipeWrite);
}
