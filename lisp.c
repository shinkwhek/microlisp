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
  int  type;
  void *car;
  void *cdr;
} SExpr;

struct Env;

typedef struct SExpr *primFUN(struct Env *_env , struct SExpr *_args);

typedef struct Env {
  int        type;
  primFUN    *fn;
  void       *car;
  void       *cdr;
  struct Env *next;
} Env;

static SExpr *NIL;
static SExpr *TRUE;
static Env   *END;

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

static void *alloe (int _typeName)
{
  Env *_new = malloc(sizeof(Env));
  _new->type = _typeName;
  _new->car  = NULL;
  _new->cdr  = NULL;
  _new->fn   = NULL;
  _new->next = NULL;
  return _new;
}

// [ToDo] GC mark-and-sweep

/**********************************************
        To create SExpr
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
  while(_str[i] != ' ' && _str[i] != ')' &&  _str[i] && _str[i] != '\0' && _str[i] != '\n'){
    r[i] = r[i] + _str[i];
    i++;
  }
  return r;
}
static int lengthOfList (SExpr *_expr)
{
  int len = 0;
  SExpr *r = _expr;
  do {
    ++len;
  } while (NIL != (r = getCdrAsCons(r)));
  return len;
}
/**********************************************
               Env
 *********************************************/
static Env *addVAR (char *varName , int value , Env *_root)
{
  Env *new = alloe(tSYM);
  new->car = malloc(sizeof(varName));
  strcpy(getCarAsString(new) , varName);
  new->cdr = malloc(sizeof(value));
  *((int *)new->cdr) = value;
  new->next = _root;
  return new;
}
static Env *addPRM (char *fnName , primFUN *_fn , Env *_root)
{
  Env *new = alloe(tPRM);
  new->car = malloc(sizeof(fnName));
  strcpy(getCarAsString(new) , fnName);
  new->fn = _fn;
  new->next = _root;
  return new;
}
static SExpr *findSYM (Env *_env , char *_obj)
{
  for (Env *env = _env ; env != END ; env = env->next){
    if ( strcmp(getCarAsString(getCarAsCons(env)) , _obj) == 0
         && _env->type == tSYM)
      return env->cdr;
  }
  return NIL;
}

static SExpr *applyPRM (Env *_env , SExpr *_obj , SExpr *_args)
{
  for (Env *env = _env ; env != END ; env = env->next){
    if ( strcmp( getCarAsString(env) , getCarAsString(_obj)) == 0 )
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
    if (strcmp( _str , getCarAsString(p) ) == 0){
      switch(p->type){
      case tPRM:
        return tPRM;
      case tSYM:
        return tSYM;
      case tFUN:
        return tFUN;
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
    }else if (isalpha(str[i]) || strchr(symbol_chars , str[i])){
      char *Token = readCharToken(&str[i]);
      switch( checkSymPrmFun(Token , _env) ){
      case tSYM:
        ret = newSYM(Token , ret);
        break;
      case tPRM:
        ret = newPRM(Token , ret);
        break;
      case tFUN:
        ret = newFUN(Token , ret);
        break;
      default:
        printf("env type error.\n");
        break;
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
    return findSYM(_env , getCarAsString(_expr));
  case tCONS:{
    SExpr *fn = eval(getCarAsCons(_expr) , _env);
    SExpr *args = getCdrAsCons(getCarAsCons(_expr));
    return apply(fn , args , _env);
  }
  default:
    return NIL;
  }
}
/**********************************************
              Primitive
 *********************************************/
// ('expr)
static SExpr *pQUOTE (Env *_env , SExpr *_expr)
{
  if (lengthOfList(_expr) != 1)
    printf("QUOTE is must 1 S-Expr.\n");
  return _expr->cdr;
}
// (+ <INT> ...
static SExpr *pADD (Env *_env , SExpr *_expr)
{
  int sum = 0;
  for (SExpr *p = _expr ; p != NIL ; p = p->cdr){
    SExpr *T = eval(p , _env);
    if (T->type != tNUM){
      printf("pADD must take number.\n");
    }else{
      sum += *getCarAsInt(T);
    }
  }
  return newNUM(sum , NULL);
}
static Env *setPRIMITIVE (Env *_env)
{
  Env *r = _env;
  r = addPRM("QUOTE" , pQUOTE , r);
  r = addPRM("+" , pADD , r);
  return r;
}
/**********************************************
              for User
 *********************************************/
static void print (SExpr *_expr)
{
  printf(";  ");
  switch (_expr->type){
  case tNUM:
    printf("<NUMBER> = %d\n" , *getCarAsInt(_expr));
    return;
  case tPRM:
    printf("<PRIMITIVE>\n");
    return; 
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
    case tPRM:
      printf("%*s%d::tPRM::%s\n",nest,"",nest, getCarAsString(cons));
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

  NIL  = malloc(sizeof(void *));
  TRUE = malloc(sizeof(void *));
  END  = malloc(sizeof(void *)); 
  
  Env *env = END;

  env = setPRIMITIVE(env);
  
  char str[255];
  
  while(1){

    fgets(str,255,stdin);

    SExpr *root = parse(str , env);
    
    printCons(root,0);

    print(eval(root , env));

  }

  return 0;
}
