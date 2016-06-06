/* 
 * Shin KAWAHARA
 * AVR LISP
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAR(expr) (Expr *)(expr->_car)
#define CDR(expr) (Expr *)(expr->_cdr)

typedef enum {
    tNIL = 1,
    tATOM,
    tSYMBOL,
    tCONS,
    tOPT
} Type;

struct Expr;

typedef struct Expr {
    /* Types of expr, this is tag watched eval that is function. */
    Type _type;

    // struct or data
    void * _car;

    // just struct
    struct Expr * _cdr;
} Expr;

// =============================================================
//           P A R S E R
// =============================================================
char *
reader (void)
{

}


// =============================================================
//          M A I N
// =============================================================
int
main (void)
{
    return 0;
}
