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
#include <sys/wait.h>




#define BUF_SIZE 256

#define MD5_LEN 32

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

/*
    funcio per alliberar un array d'arrays
*/

void freeTokens (char **paraules, int n_paraules);

/*
@brief Agafa un buffer i ho divideix en tokens
@param *buffer el buffer
@param *n_paraules punter que tindrà el número de paraules
@param sep el caracter que separarà les paraules
@return char** els tokens
*/

char **tokenizer (char *buffer, int *n_paraules, char sep);

/*
    funcio que printa un array d'arrays
*/

void printTokens (char **paraules, int n_paraules);

/*
    funcio que comprova si un string es un número
*/

int comprovarSiNumero(char *str);


int teCaracter(char *str, char c);

/*
    funcio que elimina un caracter de un string
*/

void eliminarCaracter(char *str, char c);

/*
    funcio que et diu la mida d'un fitxer
*/

int getSize(int fd);

/*
@brief Extreu una part d'un fitxer
@param fd el file descriptor del fitxer
@param startPos la posició de on llegirem
@param size la mida del que llegirem
@return char* el que hem llegir
*/

char* readFd(int fd,int startPos, int size);

/*
@brief calcula el MD5SUM d'un fitxer
@param *filepath el nom del fitxer
@return char* el MD5SUM
*/

char * calcularMD5SUM(const char *filepath);

/*
    Aquesta funcio crea un directori
*/

void crearDirectori(char *ruta);

/*
    Aquesta funcio borra un directori
*/

void borrarDirectori(char *ruta);
 
#endif
