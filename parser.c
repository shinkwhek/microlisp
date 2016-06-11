#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

void
tokenize (char * _str , char ** _tokens)
{
    int i = 0;
    char * olds = _str;
    char space  = ' ';

    while (space && *_str){
        while (*_str && (space != *_str)) _str++;
        *_str ^= space = *_str;
        _tokens[i] = (char *)malloc(sizeof(char *));
        strcpy(_tokens[i] , olds);
        i++;
        *_str++ ^= space;
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
