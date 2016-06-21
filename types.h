#ifndef _TYPES_H
#define _TYPES_H

#define tNIL 0
#define tNUMBER 1
#define tSYMBOL 2
#define tCONS 3
#define tPRIMITIVE 4
#define tTRUE 5
#define tFALSE 6

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

Expr *Nil   = &(Expr){tNIL};
Expr *True  = &(Expr){tTRUE};
Expr *False = &(Expr){tFALSE};

#endif // _TYPES_H
