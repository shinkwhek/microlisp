/*
 * File: type.h
 *
 * There're tiny structure to implement S-Expression.
 * This file is included all of .c files as BASIC STRUCTURE.
 *
 * */

#ifndef _TYPE_H
#define _TYPE_H

// Digging CAR Expression as "SExpr *".
#define CAR(expr) ((SExpr *)expr->_data)
// Digging CDR Expression as "SExpr *".
#define CDR(expr) ((SExpr *)expr->_next)

//--- Types ---//
#define NIL         (0)
#define NUMBER      (1)
#define OPERATION   (2)
#define CONS        (3)
//-------------//

typedef struct SExpression {
    /* Type of S-Expression.
     *  | NIL
     *  | NUMBER
     *  | OPERATION
     *  | CONS
     */
    int _type;

    /* Atom or Expression.
     *  | Number      -> Atom
     *  | OPERATION   -> Atom
     *  | Cons        -> Car Expression
     */
    void * _data;

    /* Only Expression.
     * | Cdr Expression
     * | NIL (NULL)
     */
    struct SExpression * _next;

} SExpr;

#endif // _TYPE_H
