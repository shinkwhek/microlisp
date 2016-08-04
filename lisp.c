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
const char symbol_chars[] = "!%^*-=+/<>";

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
   SExpr *_cons = (SExpr *)malloc(sizeof(SExpr));
  _cons->type = _typeName;
  _cons->car = NULL;
  _cons->cdr = NULL;
  return _cons;
}
/**********************************************
        Tiny
 *********************************************/
static SExpr *newCons (void *_car , void *_cdr)
{
  SExpr *new = alloc(tCONS);
  new->car = _car;
  new->cdr = _cdr;
  return new;
}
static SExpr *newNUM (int _value , void *_cdr)
{
  SExpr *new = alloc(tNUM);
  new->car = (int *)malloc(sizeof(int));
  *((int *)new->car) = _value;
  new->cdr = _cdr;
  return new;
}
static SExpr *newSYM (char *_name , void *_cdr)
{
  SExpr *new = alloc(tSYM);
  new->car = (char *)malloc(sizeof(char *));
  strcpy(((char *)new->car) , _name);
  new->cdr = _cdr;
  return new;
}
static SExpr *newFUN (char *_name , void *_cdr)
{
  SExpr *new = alloc(tFUN);
  new->car = (char *)malloc(sizeof(char *));
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
static SExpr *nReverse (SExpr *_expr)
{
   SExpr *ret = NIL;
  while (_expr != NIL ){
    SExpr *tmp = _expr;
    _expr = getCdrAsCons(_expr);
    tmp->cdr = ret;
    ret = tmp;
  }
  return ret;
}
static int waitBrackets (char *str)
{
  int i = 0;
  int leftCnt  = 1;
  int rightCnt = 0;
  while (leftCnt != rightCnt){
    if (str[i] == '('){ leftCnt++;  }
    if (str[i] == ')'){ rightCnt++; }
    ++i;
  }
  return i;
}
static SExpr *parse (char *str)
{
  SExpr *ret = NIL;
  int i = 0;
  while (str[i] != '\0'){
    /* ---- ---- ---- ---- ---- ---- ---- */
    if (str[i] == ' '){ /* Ignore space */
      ++i;
      continue;
    /* ---- ---- ---- ---- ---- ---- ---- */
    }else if ( str[i] == '('){ /* Init S-Expr */
      ret = newCons( parse(&str[i+1]), ret);
      i = i + waitBrackets(&str[i+1]);
    /* ---- ---- ---- ---- ---- ---- ---- */
    }else if ( str[i] == ')'){ /* End S-Exprs */
      return nReverse(ret);
    /* ---- ---- ---- ---- ---- ---- ---- */
    }else if ( isdigit(str[i])){ /* Make S-Expr of Number */
      ret = newNUM( readChar2Int(&str[i]), ret);
      while( str[i] != ' ' && str[i] != ')' && str[i]){i++;}
      if (str[i] == ')'){break;}
    /* ---- ---- ---- ---- ---- ---- ---- */
    }else if ( strchr(symbol_chars, str[i]) ){/* Make S-Expr of Symbol */
      ret = newSYM ( readCharToken(&str[i]), ret);
      while(str[i] != ' ' && str[i] != ')' && str[i]){i++;}
      if (str[i] == ')'){break;}
      /* ---- ---- ---- ---- ---- ---- ----*/
    }else if ( isalpha(str[i]) ){ /* Make S-Expr of Function */
      ret = newFUN ( readCharToken(&str[i]), ret);
      while(str[i] != ' ' && str[i] != ')' && str[i]){i++;}
      if (str[i] == ')'){break;}
    /* ---- ---- ---- ---- ---- ---- ---- */
    }else{
      printf("parser error.\n");
    }
    i++;
  }
  return nReverse(ret);
}
/**********************************************
               Eval
 *********************************************/

/**********************************************
              Debug
 *********************************************/
static void printCons (SExpr *cons, int nest)
{
  /* Nil obj */
  if(cons == NIL || !cons){
    printf("%*s%d::NIL \n",nest,"",nest);
    return;
    
  }else{
    switch (cons->type){
    case tNUM:
      printf("%*s%d::tNUM::%d\n",nest,"",nest, *getCarAsInt(cons));
      printCons( getCdrAsCons(cons) , nest);
      break;
    case tSYM:
      printf("%*s%d::tSYM::%s\n",nest,"",nest, getCarAsChar(cons));
      printCons( getCdrAsCons(cons) , nest);
      break;
    case tFUN:
      printf("%*s%d::tFUN::%s\n",nest,"",nest, getCarAsChar(cons));
      printCons( getCdrAsCons(cons) , nest);
      break;
    case tCONS:
       printCons( getCarAsCons(cons), nest + 1);
       printCons( getCdrAsCons(cons), nest);
       break;
    default:
      printf("print error.\n");
    }
  }
}
/**********************************************
               Main Loop
 *********************************************/
int main (void)
{
  char str[255];

  while(1){

    fgets(str,255,stdin);

    SExpr *root = parse(str);

    printCons(root,0);
  }

  return 0;
}
