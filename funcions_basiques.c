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




void freeTokens (char **paraules, int n_paraules){

    for (int i = 0; i < n_paraules; i++) free(paraules[i]);
    free(paraules);
}

void printTokens (char **paraules, int n_paraules){

    printString(1, "paraules:\n");

    for (int i = 0; i < n_paraules; i++){

        char *buffer;
        asprintf(&buffer, "\t[%s]\n", paraules[i]);
        printString(1,buffer);
        free(buffer);
    }
}

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

int comprovarSiNumero(char *str){
    int i=0;
    int esDigit=1;
    while(str[i]!='\0' ){
        if(!(str[i] >='0' && str[i]<='9')){
            esDigit=0;
            break;
        }
        i++;
    }
    return esDigit;

}

int teCaracter(char *str, char c){
    int i=0;
    int trobat=0;
    while(str[i]!='\0' ){
        if(str[i]==c){
            trobat=1;
            break;
        }
        i++;
    }
    return trobat;
}
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


int getSize(int fd){
    return lseek(fd, 0, SEEK_END);
}

