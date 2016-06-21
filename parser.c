#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "types.h"

void tokenize (char *_str , char **_tokens)
{
  int i = 0;
  char *olds = _str;
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

/**********************************************
 **                 tiny                     **
 *********************************************/
Expr *alloc (int _type)
{
  /* GCなし \(^o^)/ */
  Expr *r = (Expr *)malloc(sizeof(Expr));
  r->type = _type;
  r->atom = NULL;
  r->next = NULL;

  return r;
}

Expr *makeNUMBER (char _value){
  Expr *expr = alloc(tNUMBER);
  int *p = (int *)malloc(sizeof(int));
  *p = atoi(&_value);
  expr->atom = p;
  return expr;
}
/*
Expr **parse (char **tokens)
{
  Expr **OUT = NULL;
  int i = 0;
  while (tokens[i]){
    if (isdigit(tokens[i])){
      OUT[i] = 
    }
    
  }
}

*/
