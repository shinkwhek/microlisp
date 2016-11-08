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

const char symbol_chars[] = "!%^*-=+/\\<>";

#define tNIL  (0)
#define tNUM  (1)
#define tSYM  (2)
#define tFUN  (3)
#define tPRM  (4)
#define tCONS (5)

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
  SExpr      *head;
  SExpr      *body;
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
  SExpr *_newSExpr = malloc(sizeof(SExpr));
  _newSExpr->type = _typeName;
  _newSExpr->car = NULL;
  _newSExpr->cdr = NULL;
  return _newSExpr;
}

static void *alloe (int _typeName)
{
  Env *_newEnv = malloc(sizeof(Env));
  _newEnv->type = _typeName;
  _newEnv->fn   = NULL;
  _newEnv->head  = NULL;
  _newEnv->body  = NULL;
  _newEnv->next = NULL;
  return _newEnv;
}
// [ToDo] GC mark-and-sweep

/**** **** **** **** **** **** **** **** ****
        To create SExpr
 **** **** **** **** **** **** **** **** ****/

// (_car . _cdr).
static SExpr *cons (void *_car , void *_cdr)
{
  SExpr *_newCons = alloc(tCONS);
  _newCons->car = _car;
  _newCons->cdr = _cdr;
  return _newCons;
}
static SExpr *newNUM (int _value , void *_cdr)
{
  SExpr *_newCons = alloc(tNUM);
  _newCons->car = malloc(sizeof(_value));
  *getCarAsInt(_newCons) = _value;
  _newCons->cdr = _cdr;
  return _newCons;
}
static SExpr *newSPF (char *_name , void *_cdr , int _typename){
  int _type;
  if (_typename == tSYM)
    _type = tSYM;
  else if (_typename == tPRM)
    _type = tPRM;
  else if (_typename == tFUN)
    _type = tFUN;
  else
    _type = tNIL;
  SExpr *_newCons = alloc(_type);
  _newCons->car = malloc(sizeof(_name));
  strcpy(getCarAsString(_newCons) , _name);
  _newCons->cdr = _cdr;
  return _newCons;
}
static int readChar2Int (char *_str)
{
  int i   = 0;
  int out = 0;
  if ( _str[0] == '0' && _str[1] == 'b' ){
    i = 3;
    //while(){
    //}
  }else{
    while(0 <= (_str[i]-'0') && (_str[i]-'0') <= 9){
      out = out*10 + (_str[i]-'0');
      ++i;
    }
  }
  return out;
}
static char *readCharToken (char *_str)
{
  int i = 0;
  char *out = (char *)malloc(sizeof(char *));
  while(_str[i] != ' '&& _str[i] != ')' && _str[i] && _str[i] != '\0' && _str[i] != '\n'){
    out[i] = out[i] + _str[i];
    i++;
  }
  return out;
}
static int lengthOfList (SExpr *_expr)
{
  int len = 0;
  for (SExpr *r = _expr ; r != NIL && r != NULL ; r = r->cdr) {
    ++len;
  }
  return len;
}


/**** **** **** **** **** **** **** **** ****
               Parser
 **** **** **** **** **** **** **** **** ****/
static SExpr *nReverse (SExpr *_expr)
{
  SExpr *r = NIL;
  while (_expr != NIL ){
    SExpr *tmp = _expr;
    _expr = getCdrAsCons(_expr);
    tmp->cdr = r;
    r = tmp;
  }
  return r;
}
static int waitBrackets (char *str)
{
  int i    = 0;
  int lCnt = 1;
  int rCnt = 0;
  while (lCnt != rCnt){
    if (str[i] == '('){ lCnt++; }
    if (str[i] == ')'){ rCnt++; }
    ++i;
  }
  return i;
}

static int checkSymPrmFun (char *_str , Env *_env)
{
  for (Env *p = _env ; p != END ; p = p->next){
    if (strcmp( _str , getCarAsString(p->head) ) == 0){
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

static SExpr *parse (char *str , Env *_env)
{
  SExpr *r = NIL;
  int i = 0;
  while (str[i] != '\0' && str[i]){
    /* ---- ---- ---- Ignore space ---- ---- ---- */
    if (str[i] == ' '){
      ++i;
      continue;
    /* ---- ---- ---- Init Cons ---- ---- ---- */
    }else if ( str[i] == '('){
      r = cons( parse(&str[i+1] , _env), r);
      i = i + waitBrackets(&str[i+1]);
    /* ---- ---- ---- End Cons ---- ---- ---- */
    }else if ( str[i] == ')'){
      return nReverse(r);
    /* ---- ---- ---- S-Expr of Number ---- ---- ---- */
    }else if ( isdigit(str[i])){
      r = newNUM( readChar2Int(&str[i]), r);
      while( str[i] != ' ' && str[i] != ')' && str[i]){i++;}
      if (str[i] == ')'){break;}
    /* ---- ---- ---- S-Expr of Symbol | Function | Primitive ---- ---- ---- */
    }else if (isalpha(str[i]) || strchr(symbol_chars , str[i])){
      char *Token = readCharToken(&str[i]);
      switch( checkSymPrmFun(Token , _env) ){
      case tSYM:
        r = newSPF(Token , r , tSYM);
        break;
      case tPRM:
        r = newSPF(Token , r , tPRM);
        break;
      case tFUN:
        r = newSPF(Token , r , tFUN);
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
  return nReverse(r);
}
/**** **** **** **** **** **** **** **** ***
                    Env
 **** **** **** **** **** **** **** **** ****/
static SExpr *eval (SExpr*, Env**);

static void addVAR (char *_vname , SExpr *_val , Env **_root , int _typename)
{
  int type = tSYM;
  Env *_new = alloe(type);
  if (_typename == tSYM){
    _new->head = newSPF(_vname , _val , tSYM);
  }
  _new->next = *_root;
  *_root = _new;
}

static Env* addListVAR (SExpr *_vnameList , SExpr *_valList , Env **_root)
{
  Env *_new = *_root;
  for (SExpr *vnameList = _vnameList, *valList = _valList; vnameList != NIL; vnameList = vnameList->cdr, valList = valList->cdr) {
    Env *tmp = alloe(tSYM);
    tmp->head = newSPF(getCarAsString(vnameList) , eval(valList , &_new) , tSYM);
    tmp->next = _new;
    _new = tmp;
  }
  return _new;
}

/**** **** **** **** **** **** **** **** ****
               Eval
 **** **** **** **** **** **** **** **** ****/


static SExpr *findSPF (Env *_env , char *_name , int _typename)
{
  for (Env *e = _env ; e != END ; e = e->next){
    if (e->type == _typename && strcmp(getCarAsString(e->head) , _name) == 0){
      return getCdrAsCons(e->head);
    }
  }
  printf("dont find spf.");
  return NIL;
}

static SExpr *apply (SExpr *_expr , SExpr *_args , Env **_env)
{
  switch(_expr->type){
  case tPRM:{
    for (Env *e = *_env ; e != END ; e = e->next){
      if (e->type == tPRM && strcmp(getCarAsString(e->head),getCarAsString(_expr)) == 0)
        return e->fn(_env , _args);
    }
    printf("Don't find in Env.\n");
    return NIL;
  }
  case tFUN:{
    SExpr *symbols = _expr->car;
    SExpr *body    = _expr->cdr;
    Env *tmpEnv = *_env;
    tmpEnv = addListVAR(symbols , _args , &tmpEnv);
    return eval( body , &tmpEnv );
  }  
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
    SExpr* a = findSPF(*_env , getCarAsString(_expr) , tSYM);
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
static void defPRM (char *fnName , primFUN *_fn , Env **_root)
{
  Env *_new = alloe(tPRM);
  _new->head = newSPF(fnName,NIL,tPRM);
  _new->fn = _fn;
  _new->next = *_root;
  *_root = _new;
}

// ('expr)
static SExpr *pQUOTE (Env **_env , SExpr *_expr)
{
  if (lengthOfList(_expr) != 1)
    printf("QUOTE is must 1 S-Expr.\n");
  return _expr;
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
  return newNUM(result , NIL);
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
  return newNUM(result , NIL);
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
  return newNUM(result , NIL);
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
  return newNUM(result , NIL);
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
  return cons(A , B);
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
// (define <STMBOL> body)
static SExpr *pDefine (Env **_env , SExpr *_expr)
{
  char *vname  = getCarAsString(_expr);
  SExpr *body  = getCdrAsCons(_expr);
  addVAR(vname , body , _env , tSYM);
  //printf("def : %s.\n",vname);
  return body;
}
// (lambda (x ..) expr ..)
static SExpr *pLambda (Env **_env , SExpr *_expr)
{
  SExpr *_new = alloc(tFUN);
  _new->car = _expr->car; // symbols
  _new->cdr = _expr->cdr; // body
  return _new;
}
// (q) -- quit
static SExpr *pQuit (Env **_env , SExpr *_expr)
{
    printf("\nGood bye. :D\n");
    exit(1);
}

static void setPRIMITIVE (Env **_env)
{
  defPRM("quote" , pQUOTE      , _env);
  defPRM("-"     , pMinus      , _env);
  defPRM("*"     , pMultiplied , _env);
  defPRM("+"     , pPlus       , _env);
  defPRM("/"     , pDivided    , _env);
  defPRM(">"     , pGreater    , _env);
  defPRM("<"     , pLess       , _env);
  defPRM("="     , pEqual      , _env);
  defPRM("if"    , pIf         , _env);
  defPRM("cons"  , pCons       , _env);
  defPRM("car"   , pCar        , _env);
  defPRM("cdr"   , pCdr        , _env);
  defPRM("q"     , pQuit       , _env);
  defPRM("define", pDefine     , _env);
  defPRM("lambda", pLambda     , _env);
 }
/**** **** **** **** **** **** **** **** ****
              for User
 **** **** **** **** **** **** **** **** ****/
static void print (SExpr *_expr)
{
  if (_expr == NIL){
    printf("<NIL>");
  }else{
    switch (_expr->type){
    case tNUM:
      printf("<NUMBER> = %d." , *getCarAsInt(_expr));
      return;
    case tSYM:
      printf("<SYMBOL>.");
      return;
    case tPRM:
      printf("<PRIMITIVE>.");
      return;
    case tFUN:
      printf("<FUNCTION>.");
      return;
    case tCONS:
      printf("<CONS>.");
      return;
    default:{
      if (_expr == TRUE){
        printf("<BOOL> = True.");
      }else if (_expr == FALSE){
        printf("<BOOL> = False.");
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
/*
static void viewEnv (Env *_env)
{
  for (Env *r = _env; r != END; r = r->next){
    printf("type:%d  ", r->type);
    if (r->type == tPRM)
      printf("name:%s\n", getCarAsString(r->head));
    if (r->type == tSYM)
      printf("name:%s\n", getCarAsString(r->head));
  }
}
*/
/**** **** **** **** **** **** **** **** ****
               Main Loop
 **** **** **** **** **** **** **** **** ****/
int main (void)
{

  NIL   = malloc(sizeof(void *));
  TRUE  = malloc(sizeof(void *));
  FALSE = malloc(sizeof(void *));
  END   = malloc(sizeof(void *)); 

  Env *env;
  env = malloc(sizeof(Env*));
  
  env = END;

  setPRIMITIVE(&env);

  char str[255];
  
  while(1){

    for (int i = 0; i < 255; i++)
      str[i] = '\0';

    printf("> ");
    fgets(str,255,stdin);

    SExpr *root = parse(str , env);
    
    // printCons(root,0);

    printf("; ");
    print( eval(root , &env) );
    printf("\n");
    
    
    
    
    printf("\n");
  }

  return 0;
}
