#ifndef _PARSER_H
#define _PARSER_H

#include "types.h"

void tokenize (char *, char **);

void freeTokenize (char **);

Expr *makeNUMBER (char);

int *charToInt (char);

#endif // _PAESER_H
