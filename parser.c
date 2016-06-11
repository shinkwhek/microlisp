#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

void
tokenize (char * _str , char ** _tokens)
{
    int i = 0;
    char * olds = _str;
    char sym  = ' ';

    while (sym && *_str){
        while (*_str && (sym != *_str)) _str++;
        *_str ^= sym = *_str;
        _tokens[i] = (char *)malloc(sizeof(char *));
        strcpy(_tokens[i] , olds);
        i++;
        *_str++ ^= sym;
        olds = _str;
    }
    _tokens[i] = NULL;
}

void
freeTokenize (char ** _tokens)
{
    int i = 0;
    while ( _tokens[i] != NULL ){
        free(_tokens[i]);
        i++;
    }
}
