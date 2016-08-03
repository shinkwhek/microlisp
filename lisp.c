/**********************************************
   Lisp on AVR

   Author : Shin KAWAHARA

   License : read LICENSE
 *********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**********************************************
               TYPES
 *********************************************/
const int MAX_FUNC_NAME = 10;

enum {
  tNIL = 0,
  tNUM,
  tSYM,
  tFUN,
  tCONS
};

typedef struct SExpr {
  int type;
  void *car;
  void *cdr;
} SExpr;

static SExpr *NIL = &(SExpr){tNIL};

#define getCarAsInt(_expr)  ((int*)_expr->car)
#define getCarAsChar(_expr) ((char*)_expr->car)
#define getCarAsCons(_expr) ((SExpr*)_expr->car)
#define getCdrAsCons(_expr) ((SExpr*)_expr->cdr)
/**********************************************
               Memory manegement
 *********************************************/
static SExpr *alloc (int _typeName)
{
  /* GCなし! /(^o^)\ */
  SExpr *_cons = (SExpr *)malloc(sizeof(SExpr));
  _cons->type = _typeName;
  _cons->car = NULL;
  _cons->cdr = NULL;
  return _cons;
}
static SExpr *newNUM (int _value , void *_cdr)
{
  SExpr *new = alloc(tNUM);
  *((int *)new->car) = _value;
  new->cdr = _cdr;
  return new;
}
static SExpr *newSYM (char *_name , void *_cdr)
{
  SExpr *new = alloc(tSYM);
  strcpy(((char *)new->car) , _name);
  new->cdr = _cdr;
  return new;
}
static SExpr *newFUN (char *_name , void *_cdr)
{
  SExpr *new = alloc(tFUN);
  strcpy(((char *)new->car) , _name);
  new->cdr = _cdr;
  return new;
}
static int readChar2Int (char *_str)
{
  int i = 0;
  int out = 0;
  while(0 <= (_str[i]-'0') && (_str[i]-'0') <= 9){
    out = out*10 + (_str[i]-'0');
    ++i;
  }
  return out;
}
static char *readCharToken (char *_str)
{
  int i = 0;
  char *r = (char *)malloc(sizeof(char *));
  while(_str[i] != ' ' && _str[i] != ')' && _str[i]){
    r[i] = r[i] + _str[i];
    i++;
  }
  return r;
}
/**********************************************
               Env
 *********************************************/

/**********************************************
               Parser
 *********************************************/

/**********************************************
               Eval
 *********************************************/

/**********************************************
               Main Loop
 *********************************************/
int main (void)
{

  return 0;
}
