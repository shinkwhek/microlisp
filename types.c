#include <stdlib.h>
#include <string.h>

#include "types.h"

Expr *
getCarAsExpr (Expr * _expr)
{
    if (_expr->type == eCONS) {
        return (Expr *)_expr->atom;
    }else{
        return NULL;
    }
}

Expr *
getCdrAsExpr (Expr * _expr)
{
    if (_expr->type == eNIL){
        return NULL;
    }else{
        return _expr->next;
    }
}

