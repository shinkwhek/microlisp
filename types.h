#ifndef _TYPES_H
#define _TYPES_H

enum {
  tNIL = 1,
  tNUMBER,
  tSYMBOL,
  tCONS,
  tPRIMITIVE,
  tTRUE,
  tFALSE
};
  
typedef struct Expr {
  /*
   * Expr := Atom | Expr
   */

  int type;

  /* atom := NIL | NUMBER | SYMBOL | CAR */
  void *atom;

  /* next := CDR */
  struct Expr *next;

} Expr;

#endif // _TYPES_H
