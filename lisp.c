/**** **** **** **** **** **** **** **** ****
   Lisp on AVR

   Author : Shin KAWAHARA

   License : read LICENSE
 **** **** **** **** **** **** **** **** ****/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**** **** **** **** **** **** **** **** ****
               TYPES
 **** **** **** **** **** **** **** **** ****/
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

typedef struct SExpr *primFUN(struct Env **_env , struct SExpr *_args);

typedef struct Env {
  int        type;
  primFUN    *fn;
  void       *car;
  void       *cdr;
  struct Env *next;
} Env;

static SExpr *NIL;
static SExpr *TRUE;
static SExpr *FALSE;
static Env   *END;

#define getCarAsInt(_expr)    ((int*)_expr->car)
#define getCarAsString(_expr) ((char*)_expr->car)
#define getCarAsCons(_expr)   ((SExpr*)_expr->car)
#define getCdrAsCons(_expr)   ((SExpr*)_expr->cdr)
/**** **** **** **** **** **** **** **** ****
               Memory manegement
 **** **** **** **** **** **** **** **** ****/
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

/**** **** **** **** **** **** **** **** ****
        To create SExpr
 **** **** **** **** **** **** **** **** ****/
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
static SExpr *newSPF (char *_name , void *_cdr , int type){
  int __type;
  switch(type){
  case tSYM:
    __type = tSYM;
    break;
  case tPRM:
    __type = tPRM;
    break;
  case tFUN:
    __type = tFUN;
    break;
  }
  SExpr *new = alloc(__type);
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
  for (SExpr *r = _expr ; r != NIL && r != NULL ; r = r->cdr) {
    ++len;
  }
  return len;
}
/**** **** **** **** **** **** **** **** ***
               Env
 **** **** **** **** **** **** **** **** ****/
static void addVAR (char *envName , SExpr *val , Env **_root)
{
  Env *new = alloe(tSYM);
  new->car = malloc(sizeof(envName));
  strcpy(getCarAsString(new) , envName);
  new->cdr = val;
  new->next = (*_root);
  (*_root) = new;
}
/*
static void addFUN (char *funName , SExpr *_expr , Env **_root)
{
  Env *new = alloe(tFUN);
  new->car = malloc(sizeof(funName));
  strcpy(getCarAsString(new) , funName);
  new->cdr = malloc(sizeof(SExpr *));
  new->cdr = _expr;
  new->next = (*_root);
  (*_root) = new;
}
*/
static void addPRM (char *fnName , primFUN *_fn , Env **_root)
{
  Env *new = alloe(tPRM);
  new->car = malloc(sizeof(fnName));
  strcpy(getCarAsString(new) , fnName);
  new->fn = _fn;
  new->next = (*_root);
  (*_root) = new;
}
static SExpr *findSYM (Env **_env , char *_name)
{
  for (Env *env = (*_env) ; env != END ; env = env->next){
    if (env->type == tSYM &&
        strcmp(getCarAsString(env) , _name) == 0){
      return getCdrAsCons((*_env));
    }
  }
  return NIL;
}

static SExpr *applyPRM (Env **_env , SExpr *_obj , SExpr *_args)
{
  for (Env *env = (*_env) ; env != END ; env = env->next){
    if ( strcmp( getCarAsString(env) , getCarAsString(_obj)) == 0 )
      return env->fn(&env , _args);
  }
  return NIL;
}
/**** **** **** **** **** **** **** **** ****
               Parser
 **** **** **** **** **** **** **** **** ****/
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
  int lCnt  = 1;
  int rCnt = 0;
  while (lCnt != rCnt){
    if (str[i] == '('){ lCnt++; }
    if (str[i] == ')'){ rCnt++; }
    ++i;
  }
  return i;
}

static int checkSymPrmFun (char *_str , Env **_env)
{
  for (Env *p = (*_env) ; p != END ; p = p->next){
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
  return tSYM;
}

static SExpr *parse (char *str , Env **_env)
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
        ret = newSPF(Token , ret , tSYM);
        break;
      case tPRM:
        ret = newSPF(Token , ret , tPRM);
        break;
      case tFUN:
        ret = newSPF(Token , ret , tFUN);
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
/**** **** **** **** **** **** **** **** ****
               Eval
 **** **** **** **** **** **** **** **** ****/
static SExpr *eval (SExpr*, Env**);

static SExpr *apply (SExpr *_expr , SExpr *_args , Env **_env)
{
  switch(_expr->type){
  case tPRM:
    return applyPRM(_env , _expr , _args);
    break;
  }
  return NIL;
}

static SExpr *eval (SExpr *_expr , Env **_env)
{
  switch (_expr->type){
  case tNIL:
  case tNUM:
  case tPRM:
  case tFUN:
    return _expr;
  case tSYM:{
    SExpr* a = findSYM(_env , getCarAsString(_expr));
    return eval(a , _env);
  }
  case tCONS:{
    SExpr *expr = eval(getCarAsCons(_expr) , _env);
    SExpr *args = getCdrAsCons(getCarAsCons(_expr));
    return apply(expr , args , _env);
  }
  default:
    return NIL;
  }
}
/**** **** **** **** **** **** **** **** ****
              Primitive
 **** **** **** **** **** **** **** **** ****/
// ('expr)
static SExpr *pQUOTE (Env **_env , SExpr *_expr)
{
  if (lengthOfList(_expr) != 1)
    printf("QUOTE is must 1 S-Expr.\n");
  return _expr->cdr;
}

// (+ <NUMBER> ...
static SExpr *pPlus (Env **_env , SExpr *_expr)
{
  int result = 0;
  for (SExpr *p = _expr ; p != NIL ; p = p->cdr){
    SExpr *T = eval(p , _env);
    if (T->type != tNUM){
      printf("pPlus must take number.\n");
    }else{
      result += *getCarAsInt(T);
    }
  }
  return newNUM(result , NULL);
}
// (- <NUMBER> ...
static SExpr *pMinus (Env **_env , SExpr *_expr)
{
  int result;
  switch(lengthOfList(_expr)){
  case 0:
    result = 0;
    break;
  case 1:{
    SExpr *T = eval(_expr , _env);
    result = -(*getCarAsInt(T));
    break;
  }
  default:{
    result = *getCarAsInt(_expr);
    for (SExpr *p = getCdrAsCons(_expr) ; p != NIL ; p = p->cdr){
      SExpr *T = eval(p , _env);
      if (T->type != tNUM){
        printf("pMinus must take number.\n");
      }else{
        result -= *getCarAsInt(T);
      }
    }
  }
  }
  return newNUM(result , NULL);
}
// (* <NUMBER> ...
static SExpr *pMultiplied (Env **_env , SExpr *_expr)
{
  int result = 1;
  for (SExpr *p = _expr ; p != NIL ; p = p->cdr){
    SExpr *T = eval(p , _env);
    if (T->type != tNUM){
      printf("pMultiplied must take number.\n");
    }else{
      result *= *getCarAsInt(T);
    }
  }
  return newNUM(result , NULL);
}
// (/ <NUMBER> ...
static SExpr *pDivided (Env **_env , SExpr *_expr)
{
  int result = *getCarAsInt(_expr);
  for (SExpr *p = getCdrAsCons(_expr) ; p != NIL ; p = p->cdr){
    SExpr *T = eval(p , _env);
    if (T->type != tNUM){
      printf("pDivided must take number.\n");
    }else if(*getCarAsInt(T) == 0){
      printf("Cannot divided by zero.\n");
    }else{
      result /= *getCarAsInt(T);
    }
  }
  return newNUM(result , NULL);
}
// (> A B)
static SExpr *pGreater (Env **_env , SExpr *_expr)
{
  SExpr *A = eval(_expr               , _env);
  SExpr *B = eval(getCdrAsCons(_expr) , _env);
  if (*getCarAsInt(A) > *getCarAsInt(B)){
    return TRUE;
  }else if ( !(*getCarAsInt(A) > *getCarAsInt(B)) ){
    return FALSE;
  }else{
    printf("pGreater error.\n");
  }
  return NIL;
}
// (< A B)
static SExpr *pLess (Env **_env , SExpr *_expr)
{
  SExpr *A = eval(_expr               , _env);
  SExpr *B = eval(getCdrAsCons(_expr) , _env);
  if (*getCarAsInt(A) < *getCarAsInt(B)){
    return TRUE;
  }else if ( !(*getCarAsInt(A) < *getCarAsInt(B)) ){
    return FALSE;
  }else{
    printf("pLess error.\n");
  }
  return NIL;
}
// (= A B)
static SExpr *pEqual (Env **_env , SExpr *_expr)
{
  SExpr *A = eval(_expr               , _env);
  SExpr *B = eval(getCdrAsCons(_expr) , _env);
  if (*getCarAsInt(A) == *getCarAsInt(B)){
    return TRUE;
  }else if( !(*getCarAsInt(A) == *getCarAsInt(B)) ){
    return FALSE;
  }else{
    printf("pEqual error.\n");
  }
  return NIL;
}
// (if (TRUE | FALSE) A B)
static SExpr *pIf (Env **_env , SExpr *_expr)
{
  SExpr *P = eval(_expr , _env); // TRUE | FALSE
  if (P == TRUE){
    return eval(getCdrAsCons(_expr) , _env);
  }else if (P == FALSE){
    return eval(getCdrAsCons(getCdrAsCons(_expr)) , _env);
  }else{
    printf("pIf error.\n");
  }
  return NIL;
}
// (cons A B) -> (A . B)
static SExpr *pCons (Env **_env , SExpr *_expr)
{
  SExpr *A = _expr;
  SExpr *B = getCdrAsCons(_expr);
  return newCons(A , B);
}
// (car (A _)) -> A
static SExpr *pCar (Env **_env , SExpr *_expr)
{
  SExpr *A = getCarAsCons(_expr);
  return A;
}
// (cdr (A _)) -> _
static SExpr *pCdr (Env **_env , SExpr *_expr)
{
  SExpr *O = getCdrAsCons(getCarAsCons(_expr));
  return O;
}
// (defn X Y) -> variable | (defn (X Y) Z) -> Function
static SExpr *pDefn (Env **_env , SExpr *_expr)
{
  if (_expr->type == tCONS){
  }else{
    // variable mode
    char  *a = getCarAsString(_expr);
    SExpr *b = eval(getCdrAsCons(_expr) , _env);
    addVAR(a , b , _env);
  }
  return NIL;
}
static SExpr *pQuit (Env **_env , SExpr *_expr)
{
    printf("\nGood bye. :D\n");
    exit(1);
}

static void setPRIMITIVE (Env **_env)
{
  addPRM("QUOTE" , pQUOTE      , _env);
  addPRM("+"     , pPlus       , _env);
  addPRM("-"     , pMinus      , _env);
  addPRM("*"     , pMultiplied , _env);
  addPRM("/"     , pDivided    , _env);
  addPRM(">"     , pGreater    , _env);
  addPRM("<"     , pLess       , _env);
  addPRM("="     , pEqual      , _env);
  addPRM("if"    , pIf         , _env);
  addPRM("cons"  , pCons       , _env);
  addPRM("car"   , pCar        , _env);
  addPRM("cdr"   , pCdr        , _env);
  addPRM("defn"  , pDefn       , _env);
  addPRM("q"     , pQuit       , _env);
  addVAR("x"     , newNUM(1,NULL) , _env);
}
/**** **** **** **** **** **** **** **** ****
              for User
 **** **** **** **** **** **** **** **** ****/
static void print (SExpr *_expr)
{
  printf(";  ");

  if (_expr == NIL){
    printf("<NIL>\n");
  }else{
    switch (_expr->type){
    case tNUM:
      printf("<NUMBER> = %d.\n" , *getCarAsInt(_expr));
      return;
    case tSYM:
      printf("<SYMBOL> := %d.\n" , *getCarAsInt(_expr));
      return;
    case tPRM:
      printf("<PRIMITIVE>.\n");
      return;
    case tCONS:
      printf("(<SEXPR> . <SEXPR>)\n");
      return;
    default:{
      if (_expr == TRUE){
        printf("<BOOL> = True.\n");
      }else if (_expr == FALSE){
        printf("<BOOL> = False.\n");
      }
    }
    }
  }
}
/**** **** **** **** **** **** **** **** ****
              Debug
 **** **** **** **** **** **** **** **** ****/
/*
static void printCons (SExpr *cons, int nest)
{
   Nil obj 
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
*/
static void viewEnv (Env *_env)
{
  for (Env *r = _env; r != END; r = r->next){
    printf("type:%d  ", r->type);
    printf("name:%s\n", getCarAsString(r));
  }
}
/**** **** **** **** **** **** **** **** ****
               Main Loop
 **** **** **** **** **** **** **** **** ****/
int main (void)
{

  NIL   = malloc(sizeof(void *));
  TRUE  = malloc(sizeof(void *));
  FALSE = malloc(sizeof(void *));
  END   = malloc(sizeof(void *)); 
  
  Env *env = END;

  setPRIMITIVE(&env);

  char str[255];
  
  while(1){

    for (int i = 0; i < 255; i++)
      str[i] = '\0';

    printf("> ");
    fgets(str,255,stdin);

    SExpr *root = parse(str , &env);
    
    // printCons(root,0);

    print( eval(root , &env) );
    viewEnv(env);
    // freeSEXPR(getCarAsCons(root));
    // freeSEXPR(getCdrAsCons(root));

    // freeSEXPR(root);
    
    printf("\n");
  }

  return 0;
}
