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
