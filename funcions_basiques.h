#ifndef _FUNCBASIQUES_H_
#define _FUNCBASIQUES_H_

#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>


#define BUF_SIZE 256


/*
    funcio per printar un sol caracter
*/

void printChar (int fileDescriptor, char prCh);
/*
    funcio per printar un array de caracters
*/

void printString (int fileDescriptor, char *prStr);

/*
    funcio per printar un integer
*/

void printUInt (int fileDescriptor, unsigned int integer);

/*
    funcio per llegir d'un fitxer fins arribar a un caracter específic
*/

char* readUntil (int fileDescriptor, char stop, int *size);

/*
    funcio per llegir d'un fitxer fins arribar a un caracter específic, però sense treure
    res, es nomes per moure l'index.  (lseek + stop)
*/

void moveReadIdexUntil (int fileDescriptor, char stop, int *size);

/*
    funcio per comparar només una part d'un string amb un altre string
*/

char substrCmp (char *str, int start, int end, char *str2);

/*
    funcio que cambia el primer \n per \0 en un string
*/

void removeFirstEnter (char (*str) [], int size);


void freeTokens (char **paraules, int n_paraules);
char **tokenizer (char *buffer, int *n_paraules, char sep);
void printTokens (char **paraules, int n_paraules);


int comprovarSiNumero(char *str);
int teCaracter(char *str, char c);
void eliminarCaracter(char *str, char c);


int getSize(int fd);

char* llegirComanda(int *exitflag) ;

#endif
