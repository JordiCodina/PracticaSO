#include "funcions_basiques.h"

/*
    funcio per printar un sol caracter
*/

void printChar (int fileDescriptor, char prCh){
    write (fileDescriptor, &prCh, 1);
}

/*
    funcio per printar un array de caracters
*/

void printString (int fileDescriptor, char *prStr){
    write (fileDescriptor, prStr, strlen(prStr));
}

/*
    funcio per printar un integer
*/

void printUInt (int fileDescriptor, unsigned int integer){
    char *buffer;
    asprintf(&buffer, "%u", integer);
    if (buffer){
        printString (fileDescriptor, buffer);
        free(buffer);
    }
}

/*
    funcio per llegir d'un fitxer fins arribar a un caracter específic
*/

char* readUntil (int fileDescriptor, char stop, int *size){

    (*size) = 0;
    char *string = NULL;
    char newChar = '\0';
    int hasRead = 0;
    hasRead = read (fileDescriptor, &newChar, 1);

    while ((hasRead) && (newChar != stop)){

        string = realloc (string, ++(*size) * sizeof(char));
        string [(*size) - 1] = newChar;

        hasRead = read (fileDescriptor, &newChar, 1);
    }

    if ((*size) == 0) {
        if (string != NULL) free(string);
        return NULL;
    }

    string = realloc (string, ((*size) + 1) * sizeof(char));
    string[(*size)] = '\0';

    return string;
}

/*
    funcio per llegir d'un fitxer fins arribar a un caracter específic, però sense treure
    res, es nomes per moure l'index.  (lseek + stop)
*/

void moveReadIdexUntil (int fileDescriptor, char stop, int *size){

    char *string = readUntil (fileDescriptor, stop, size);
    if (string != NULL) free(string);

}

/*
    funcio per comparar només una part d'un string amb un altre string
*/

char substrCmp (char *str, int start, int end, char *str2){

    char *strsub = malloc (sizeof(char) * (end - start + 1));

    for (int i = start; i < end; i++) strsub[i - start] = str[i];
    strsub[end - start] = '\0';

    //char *buffer;
    //asprintf(&buffer, "\n[%s][%s]\n", strsub, str2);
    //printString(1,buffer);
    //free(buffer);
    char result = strcasecmp(strsub, str2);

    free(strsub);
    return result;
}

/*
    funcio que cambia el primer \n per \0 en un string
*/

void removeFirstEnter (char (*str) [], int size){

    for (int i = 0; i < size; i++){
        if ((*str)[i] == '\n'){
            (*str)[i] = '\0';
            return;
        }
    }
}

/*
    funcio per alliberar un array d'arrays
*/

void freeTokens (char **paraules, int n_paraules){

    for (int i = 0; i < n_paraules; i++) free(paraules[i]);
    free(paraules);
}

/*
@brief Agafa un buffer i ho divideix en tokens
@param *buffer el buffer
@param *n_paraules punter que tindrà el número de paraules
@param sep el caracter que separarà les paraules
@return char** els tokens
*/

char **tokenizer (char* buffer, int *n_paraules,char sep){

    (*n_paraules) = 0;
    char **paraules = NULL;
    int i = 0;

    while (buffer[i] != '\0') {
        while (buffer[i] == sep){
            i++;
        }
        if (buffer[i] == '\0') break;
        //printString (1, "\tstartWord\n");
        int p_size = 0;
        paraules = (char **) realloc (paraules, (++*n_paraules) * sizeof(char *));
        paraules[(*n_paraules) - 1] = NULL;

        do {
 
            //printChar(1, '[');
            //printChar(1, buffer[i]);
            //printChar(1, ']');
            p_size++;
            paraules[(*n_paraules) - 1] = (char *) realloc (paraules[(*n_paraules) - 1], p_size * sizeof(char));
            paraules[(*n_paraules) - 1][p_size - 1] = buffer[i];
            i++;
        } while ((buffer[i] !=  sep) && (buffer[i] != '\0'));

        //printString (1, "TEST1.4 ");

        paraules[(*n_paraules) - 1] = realloc (paraules[(*n_paraules) - 1], sizeof(char) * (1 + p_size));
        paraules[(*n_paraules) - 1][p_size] = '\0';

        //printString (1, "\tendWord\n");
    }

    return paraules;
}

/*
    funcio que printa un array d'arrays
*/

void printTokens (char **paraules, int n_paraules){

    printString(1, "paraules:\n");

    for (int i = 0; i < n_paraules; i++){

        char *buffer;
        asprintf(&buffer, "\t[%s]\n", paraules[i]);
        printString(1,buffer);
        free(buffer);
    }
}

/*
    funcio que comprova si un string es un número
    1 = si, 0 = no
*/

int comprovarSiNumero(char *str){
    int i=0;

    while(str[i]!='\0' ){
        if(!(str[i] >='0' && str[i]<='9')) return 0
        i++;
    }
    return 1;

}

/*
    funcio que mira si un string te un caracter
    1 = si, 0 = no
*/

int teCaracter(char *str, char c){
    int i=0;

    while(str[i]!='\0' ){
        if(str[i]==c){
            return 1;
        }
        i++;
    }
    return 0;
}

/*
    funcio que elimina un caracter de un string
*/

void eliminarCaracter(char *str, char c) {
    if (str == NULL) {
        return;
    }
    int i = 0; 
    int j = 0; 
    while (str[i]!='\0') {
        if (str[i]!=c) {
            str[j]=str[i];
            j++;
        }
        i++;
    }
    str[j]='\0';
}

/*
    funcio que et diu la mida d'un fitxer
*/

int getSize(int fd){
    return lseek(fd, 0, SEEK_END);
}

/*
@brief Extreu una part d'un fitxer
@param fd el file descriptor del fitxer
@param startPos la posició de on llegirem
@param size la mida del que llegirem
@return char* el que hem llegir
*/

char* readFd(int fd,int startPos, int size){
    lseek(fd, startPos, SEEK_SET);
    char *buffer=malloc((size+1)*sizeof(char));
    read(fd, buffer, size);
    buffer[size]='\0';
    //printString(1,buffer);
    return buffer;
}

/*
@brief calcula el MD5SUM d'un fitxer
@param *filepath el nom del fitxer
@return char* el MD5SUM
*/

char * calcularMD5SUM(const char *filepath) {
    int pipefd[2];
    if (pipe(pipefd) < 0) return NULL;

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        return NULL;
    }

    if (pid == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execlp("md5sum", "md5sum", filepath, NULL);
        exit(1);
    }

    close(pipefd[1]);

    char buffer[256] = {0};
    ssize_t n = read(pipefd[0], buffer, sizeof(buffer)-1);
    close(pipefd[0]);

    int status;
    waitpid(pid, &status, 0);

    if (n <= 0 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        return NULL;
    }

    // Busca el primer espai
    char *space = strchr(buffer, ' ');
    if (!space) return NULL;

    size_t len = space - buffer;
    if (len != 32) return NULL;  // longitud MD5 correcte

    char *md5 = malloc(33);
    if (!md5) return NULL;

    memcpy(md5, buffer, 32);
    md5[32] = '\0';

    return md5;
}

/*
    Aquesta funcio crea un directori
*/

void crearDirectori(char *ruta){
    char *missDir; 
    asprintf(&missDir, ".%s", ruta); 
    pid_t pid = fork();
    if (pid == 0) {
        execlp("mkdir", "mkdir", "-p", missDir, (char*)NULL);
        _exit(1);
    } else if (pid > 0) {
        waitpid(pid, NULL, 0);
    } else {
        printString(1, "Fork error\n");
        free(missDir);
        return;
    }
    free(missDir);
}

/*
    Aquesta funcio borra un directori
*/

void borrarDirectori(char *ruta){
    char *missDir;
    asprintf(&missDir, ".%s", ruta);

    pid_t pid = fork();
    if (pid == 0) {
        execlp("rm", "rm", "-rf", missDir, (char*)NULL);
        _exit(1);
    } else if (pid > 0) {
        waitpid(pid, NULL, 0);
    } else {
        printString(1, "Fork error\n");
        free(missDir);
        return;
    }

    free(missDir);
}

