#ifndef _EVAL_H
#define _EVAL_H

#include "type.h"

#define ADDVANCE    (1)
#define MULTIPLY    (2)
#define DIVIDE      (3)

// (Expr and env) -> long
long eval (SExpr **, int);

#endif // _EVAL_H
