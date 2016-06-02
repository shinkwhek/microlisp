#include <string.h>
#include <stdlib.h>
#include "type.h"

long
eval (SExpr ** expr , int env)
{
    if (expr[0]->_type == NUMBER){
        return atol(expr[0]->_data);
    }

    if (expr[0]->_type == NIL){
        return 0;
    }

    if (expr[0]->_type == OPERATION){
        int i = 1;
    }
}

/* Tiny Functions */
