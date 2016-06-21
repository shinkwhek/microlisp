#ifndef _TYPES_H
#define _TYPES_H


typedef enum {
    eERROR,
    eNIL,
    eNUM,
    eSYM,
    eCONS
} Type;


typedef struct Expr {
  /*
   * Expr := Atom | Expr
   */

  Type type;

  /* atom := NIL | NUMBER | SYMBOL | CAR */
  void * atom;

  /* next := CDR */
  struct Expr * next;
} Expr;

#endif // _TYPES_H
