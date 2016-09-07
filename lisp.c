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

enum {
  tNIL = 0,
  tNUM,
  tSYM,
  tFUN,
  tPRM,
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
  SExpr *_new = malloc(sizeof(SExpr));
  _new->type = _typeName;
  _new->car = NULL;
  _new->cdr = NULL;
  return _new;
}

static void *alloe (int _typeName)
{
  Env *_new = malloc(sizeof(Env));
  _new->type = _typeName;
  _new->fn   = NULL;
  _new->head  = NULL;
  _new->body  = NULL;
  _new->next = NULL;
  return _new;
}
// [ToDo] GC mark-and-sweep

/**** **** **** **** **** **** **** **** ****
        To create SExpr
 **** **** **** **** **** **** **** **** ****/

// (_car . _cdr).
static SExpr *cons (void *_car , void *_cdr)
{
  SExpr *_new = alloc(tCONS);
  _new->car = _car;
  _new->cdr = _cdr;
  return _new;
}
static SExpr *newNUM (int _value , void *_cdr)
{
  SExpr *_new = alloc(tNUM);
  _new->car = malloc(sizeof(_value));
  *getCarAsInt(_new) = _value;
  _new->cdr = _cdr;
  return _new;
}
static SExpr *newSPF (char *_name , void *_cdr , int _typename){
  int _type;
  switch(_typename){
  case tSYM:
    _type = tSYM;
    break;
  case tPRM:
    _type = tPRM;
    break;
  case tFUN:
    _type = tFUN;
    break;
  }
  SExpr *new = alloc(_type);
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
  while(_str[i] != ' '&& _str[i] != ')' && _str[i] && _str[i] != '\0' && _str[i] != '\n'){
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
/*
static SExpr *allElmEval (SExpr *_expr , Env **_env)
{
  SExpr *new = NIL;
  for (SExpr *p = _expr; p != NIL ; p = p->cdr){
    SExpr *T = eval(p, _env);
    T->cdr = new;
    new = T;
  }
  return nReverse(new);
}
*/
/**** **** **** **** **** **** **** **** ***
               Env
 **** **** **** **** **** **** **** **** ****/
/*
static void addVAR (SExpr *vname , SExpr *val , Env **_root)
{
  Env *new = alloe(tSYM);
  new->car = vname;
  new->cdr = val;
  new->next = (*_root);
  (*_root) = new;
}
*/
/*
static Env *addFUN (SExpr *fhead , SExpr *body , Env **_root)
{
  Env *new = alloe(tFUN);
  new->car = fhead;
  new->cdr = body;
  new->next = (*_root);
  return new;
}
*/

static SExpr *findSPF (Env **_env , char *_name , int _typename)
{
  for (Env *e = (*_env) ; e != END ; e = e->next){
    if (e->type == _typename && strcmp(getCarAsString(e->head) , _name) == 0){
      return getCdrAsCons(e->head);
    }
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

static SExpr *parse (char *str , Env **_env)
{
  SExpr *ret = NIL;
  int i = 0;
  while (str[i] != '\0' && str[i]){
    /* ---- ---- ---- Ignore space ---- ---- ---- */
    if (str[i] == ' '){
      ++i;
      continue;
    /* ---- ---- ---- Init Cons ---- ---- ---- */
    }else if ( str[i] == '('){
      ret = cons( parse(&str[i+1] , _env), ret);
      i = i + waitBrackets(&str[i+1]);
    /* ---- ---- ---- End Cons ---- ---- ---- */
    }else if ( str[i] == ')'){
      return nReverse(ret);
    /* ---- ---- ---- S-Expr of Number ---- ---- ---- */
    }else if ( isdigit(str[i])){
      ret = newNUM( readChar2Int(&str[i]), ret);
      while( str[i] != ' ' && str[i] != ')' && str[i]){i++;}
      if (str[i] == ')'){break;}
    /* ---- ---- ---- S-Expr of Symbol | Function | Primitive ---- ---- ---- */
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
  case tPRM:{
    for (Env *e = (*_env) ; e != END ; e = e->next){
      if (strcmp(getCarAsString(e->head) , getCarAsString(_expr)) == 0)
        return e->fn(&e , _args);
    }
    printf("Don't find in Env.\n");
    return NIL;
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
    SExpr* a = findSPF(_env , getCarAsString(_expr) , tSYM);
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
  _new->next = (*_root);
  (*_root) = _new;
}
/*
static void defPRM2 (char *fnName , primFUN *_fn , Env **_root)
{
  Env *new = alloe(tPRM);
  new->car = malloc(sizeof(fnName));
  strcpy(getCarAsString(new) , fnName);
  new->fn = _fn;
  new->next = (*_root);
  (*_root) = new;
}
*/
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

// (lambda (x ..) expr ..)
/*
static SExpr *pLambda (Env **_env , SExpr *_expr)
{
  SExpr *symbols = getCarAsCons(_expr);
  SExpr *exprs   = getCdrAsCons(_expr);

  SExpr *new = alloc(tFUN);
  new->car = symbols;
  new->cdr = exprs;
  return new;
}
*/
// (q) -- quit
static SExpr *pQuit (Env **_env , SExpr *_expr)
{
    printf("\nGood bye. :D\n");
    exit(1);
}

static void setPRIMITIVE (Env **_env)
{
  defPRM("QUOTE" , pQUOTE      , _env);
  defPRM("+"     , pPlus       , _env);
  defPRM("-"     , pMinus      , _env);
  defPRM("*"     , pMultiplied , _env);
  defPRM("/"     , pDivided    , _env);
  defPRM(">"     , pGreater    , _env);
  defPRM("<"     , pLess       , _env);
  defPRM("="     , pEqual      , _env);
  defPRM("if"    , pIf         , _env);
  defPRM("cons"  , pCons       , _env);
  defPRM("car"   , pCar        , _env);
  defPRM("cdr"   , pCdr        , _env);
  defPRM("q"     , pQuit       , _env);
  //defPRM("lambda", pLambda     , _env);
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
    if (r->type == tPRM)
      printf("name:%s\n", getCarAsString(r->head));
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

  Env **env;
  env = malloc(sizeof(Env**));
  
  (*env) = END;

  setPRIMITIVE(env);

  char str[255];
  
  while(1){

    for (int i = 0; i < 255; i++)
      str[i] = '\0';

    printf("> ");
    fgets(str,255,stdin);

    SExpr *root = parse(str , env);
    
    // printCons(root,0);

    print( eval(root , env) );
    //viewEnv(*env);
    // freeSEXPR(getCarAsCons(root));
    // freeSEXPR(getCdrAsCons(root));

    // freeSEXPR(root);
    
    printf("\n");
  }

  return 0;
}
