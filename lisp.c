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
  tPRM,
  tENV,
  tCONS
};

typedef struct SExpr {
  int type;
  void *car;
  void *cdr;
} SExpr;

struct Env;

typedef struct SExpr *primFUN(struct Env *_env , SExpr *_args);

typedef struct Env {
  int type;
  primFUN *fn;
  void *car;
  void *cdr;
  struct Env *next;
} Env;

static SExpr *NIL;
static Env *END;

#define getCarAsInt(_expr)    ((int*)_expr->car)
#define getCarAsString(_expr) ((char*)_expr->car)
#define getCarAsCons(_expr)   ((SExpr*)_expr->car)
#define getCdrAsCons(_expr)   ((SExpr*)_expr->cdr)
/**********************************************
               Memory manegement
 *********************************************/

static void *alloc (int _typeName)
{
  SExpr *_cons = malloc(sizeof(SExpr));
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
  new->car = malloc(sizeof(_value));
  *getCarAsInt(new) = _value;
  new->cdr = _cdr;
  return new;
}
static SExpr *newSYM (char *_name , void *_cdr)
{
  SExpr *new = alloc(tSYM);
  new->car = malloc(sizeof(_name));
  strcpy(getCarAsString(new) , _name);
  new->cdr = _cdr;
  return new;
}
static SExpr *newPRM (char *_name , void *_cdr)
{
  SExpr *new = alloc(tPRM);
  new->car = malloc(sizeof(_name));
  strcpy(getCarAsString(new) , _name);
  new->cdr = _cdr;
  return new;
}
static SExpr *newFUN (char *_name , void *_cdr)
{
  SExpr *new = alloc(tFUN);
  new->car = malloc(sizeof(_name));
  strcpy(getCarAsString(new) , _name);
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
  while(_str[i] != ' ' && _str[i] != ')' &&  _str[i] && _str[i] != '\0'){
    r[i] = r[i] + _str[i];
    i++;
  }
  return r;
}

/**********************************************
               Env
 *********************************************/
static void *findSYM (Env *_env , SExpr *_obj)
{
  for (Env *env = _env ; env != END ; env = env->next){
    if ( strcmp( ((char*)env->car) , getCarAsString(_obj)) == 0 )
      return env->cdr;
  }
  return NIL;
}
static SExpr *applyPRM (Env *_env , SExpr *_obj , SExpr *_args)
{
  for (Env *env = _env ; env != END ; env = env->next){
    if ( strcmp( ((char*)env->car) , getCarAsString(_obj)) == 0 )
      return env->fn(_env , _args);
  }
  return NIL;
}
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

static int checkSymPrmFun (char *_str , Env *_env)
{
  for (Env *p = _env ; p != END ; p = p->next){
    if (strcmp(_str , ((char *)p->car)) == 0){
      switch(p->type){
      case tPRM:
        return tPRM;
      case tSYM:
        return tSYM;
      case tFUN:
        return tFUN;
      default:
        return 0;
      }
    }
  }
  return 0;
}

static SExpr *parse (char *str , Env *_env)
{
  SExpr *ret = NIL;
  int i = 0;
  while (str[i] != '\0' && str[i]){
    /* ---- ---- ---- ---- ---- ---- ---- */
    if (str[i] == ' '){ /* Ignore space */
      ++i;
      continue;
    /* ---- ---- ---- ---- ---- ---- ---- */
    }else if ( str[i] == '('){ /* Init S-Expr */
      ret = newCons( parse(&str[i+1] , _env), ret);
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
    }else if (isalpha(str[i]) || strchr(symbol_chars, str[i])){
      char *Token = readCharToken(&str[i]);
      switch(checkSymPrmFun(Token , _env)){
      case tPRM:
        ret = newPRM( Token, ret);
        break;
      case tSYM:
        ret = newSYM( Token, ret);
        break;
      case tFUN:
        ret = newFUN( Token, ret);
        break;
      default:
        printf("okasii\n");
      }
      while(str[i] != ' ' && str[i] != ')' && str[i]){i++;}
      if (str[i] == ')'){break;}
    /* ---- ---- ---- ---- ---- ---- ---- */
    }else{
      break;
    }
    ++i;
  }
  return nReverse(ret);
}
/**********************************************
               Eval
 *********************************************/
static SExpr *eval (SExpr*, Env*);

static SExpr *apply (SExpr *_fn , SExpr *_args , Env *_env)
{
  if (_fn->type == tPRM)
    return applyPRM(_env , _fn , _args);
  return NIL;
}

static SExpr *eval (SExpr *_expr , Env *_env)
{
  switch (_expr->type){
  case tNIL:
  case tNUM:
  case tPRM:
  case tFUN:
    return _expr;
  case tSYM:
    return findSYM(_env , _expr);
  case tCONS:{
    SExpr *fn = eval(getCarAsCons(_expr) , _env);
    SExpr *args = getCarAsCons(getCdrAsCons(_expr));
    return apply(fn , args , _env);
  }
  default:
    return NIL;
  }
}
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
      printf("%*s%d::tSYM::%s\n",nest,"",nest, getCarAsString(cons));
      printCons( getCdrAsCons(cons) , nest);
      break;
    case tFUN:
      printf("%*s%d::tFUN::%s\n",nest,"",nest, getCarAsString(cons));
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

  NIL = malloc(sizeof(void *));
  END = malloc(sizeof(void *)); 
  
  //char str[255];

  Env *env = END;
  
  char str[255];
  
  while(1){

    fgets(str,255,stdin);

    SExpr *root = parse(str , env);

    printCons(root,0);

  }

  return 0;
}
