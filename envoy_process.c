
#include "envoy_process.h"


int exitEnvoyFlag = 0;

void signalHandlerEnvoy(int sig) {
    exitEnvoyFlag = 1;
}

int mirarTimeoutPledge(int mirarTimeoutFlag,double timeout, time_t inici, char *nomDest) {
        if (mirarTimeoutFlag <= 0) return 0;
        if (difftime(time(NULL), inici) >= timeout) {
            char *tmp = NULL;
            if (asprintf(&tmp, "Pledge to %s timed out.\n", nomDest) != -1) {
                printString(1, tmp);
                free(tmp);
            }
            
        //write timeout
        return 0;
    }
    return 1;
}    
void runenvoy(key_t shm_key, int pipeRead, int pipeWrite,Regne *r) {

    int statusACKFitxer=0;
    int statusACKMD5=0;
    int statuspledgeRespond=0;
    time_t inici;
     const double timeoutPledge = 120.0;
     int mirarTimeout=0;
    signal(SIGINT, signalHandlerEnvoy);

    fd_set read_set;
    struct timeval timeout;
    ForkRegneWritesEnvoy infoInicial;

    while (1) {
        if (exitEnvoyFlag) break;

        FD_ZERO(&read_set);
        FD_SET(pipeRead, &read_set);

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int ret = select(pipeRead + 1, &read_set, NULL, NULL, &timeout);

        if (ret < 0) continue;
        if(ret == 0) {
             mirarTimeout = mirarTimeoutPledge(mirarTimeout,timeoutPledge, inici, infoInicial.regneDest);
            continue;
        } // Timeout, loop again


        if (FD_ISSET(pipeRead, &read_set)) {
            ForkRegneWritesEnvoy buffer;
            int n = read(pipeRead, &buffer, sizeof(ForkRegneWritesEnvoy));
            if (n <= 0) break;

          
           
            switch (buffer.comanda) {
                case 0: // PLEDGE
                    //   
                    mirarTimeout=0;
                    
                    infoInicial=buffer;
                    pledgeEnvoyTramaInicial(infoInicial.rutaSegell, infoInicial.regneDest,r,infoInicial.indexEnvoy);

                    break;
                case 1: // LIST PRODUCTS
                    //
                    int indexRuta=-1;
                    for(int i=0;i<r->n_rutes;i++){
                        if(strcmp(r->rutes[i].nomDest,buffer.regneDest)==0){
                            indexRuta=i;
                            break;
                        }
                    }   
                    listProductsAliat(r, buffer.regneDest,indexRuta);
                    // No implementat en aquesta fase
                    break;
                case 2: // SEND PRODUCTS
                    // No implementat en aquesta fase
                    break;
                case 3: // (PLEDGE) REBRE ACK FITXER -> OK -> ENVIAR SIGIL
                    statusACKFitxer=buffer.ackStatus; // 1 = OK , 2 = KO
                    if(statusACKFitxer==1){
                        printf("Envoy PID %d received ACK FITXER OK, sending sigil...\n", getpid());
                        pledgeEnvoyEnviarSigil( infoInicial.rutaSegell, infoInicial.regneDest,r,infoInicial.indexEnvoy,&exitEnvoyFlag);
                    }else{
                        //write ackfitxer ha anat malament
                    }
                break;
                case 4:// (PLEDGE) REBRE ACK MD5SUM -> OK -> COMENÇAR 2 minuts
                    statusACKMD5=buffer.ackStatus; // 1 = OK , 2 = KO
                    if(statusACKMD5==1){
                        mirarTimeout=1;
                         inici= time(NULL); //guardem
                        printf("Envoy PID %d received ACK MD5SUM OK, starting timeout...\n", getpid());
                        
                        }else{

                        //write ackmd5 ha anat malament
                    }
                    break;
                case 5:// (PLEDGE respond)rebre pledge respond
                    
                printf("pledge respond received, case 5\n");
                    if(mirarTimeoutPledge(mirarTimeout,timeoutPledge, inici, infoInicial.regneDest)) {
                        printString(1, "Pledge respond received after timeout, ignoring...\n");
                        mirarTimeout=0;
                        if(buffer.statusPledgeRespond==1) // 1 = ACCEPT , 2 = REJECT
                        { 
                            EnvoyWritesRegne response;
                            for(int i=0;i<r->n_rutes;i++){
                                if(strcmp(r->rutes[i].nomDest,infoInicial.regneDest)==0){
                                    response.indexRuta=i;
                                    break;
                                }
                            }
                            
                            response.comanda = 1; // PLEDGE RESPONSE
                            response.statusPledge = 1; // ACCEPT
                            response.indexEnvoy = infoInicial.indexEnvoy;
                            write(pipeWrite, &response, sizeof(EnvoyWritesRegne));
                            printf("Envoy PID %d sent PLEDGE RESPOND ACCEPT, exiting...\n", getpid());

                            //w rite accept
                        }else{
                            //write reject
                        }
                    }


                    printf("Envoy PID %d received PLEDGE RESPOND %d, exiting...\n", getpid(), statuspledgeRespond);
                   
                    
                break;

                break;
                default:
                    printf("Envoy PID %d received unknown command %d\n", getpid(), buffer.comanda);
                    break;
            }

        }
    }

    printf("Envoy PID %d exiting...\n", getpid());
    close(pipeRead);
    close(pipeWrite);
}
