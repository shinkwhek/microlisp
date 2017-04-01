#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>

FILE * fp;
int c;

/* ==== ==== ==== type ==== ==== ==== */
enum {
  TNIL = 0,
  TENV,
  TTRUE,
  TFALSE,
  TSYMBOL,
  TCONS,
  TINT
};

struct cell_s;

typedef struct cell_s * (* Prim)(struct cell_s * cell, struct cell_s * env);

typedef struct cell_s {
  int type_;
  union {
	// data
	int int_;
	char * symbol_;
	struct cell_s * car_;
	// to env
	Prim * prim_;
  };
  struct cell_s * cdr_;
} Cell;

static Cell * Nil   = &(Cell){ TNIL,   .int_ = 0 };
static Cell * TRUE  = &(Cell){ TTRUE,  .int_ = 1 };
static Cell * FALSE = &(Cell){ TFALSE, .int_ = 0 };

const char symbols[] = "+-*/!?=<>_:\\%#~&";
/* ==== ==== ==== ==== ==== ==== ==== */

/* ---- ---- make cell ---- ---- */
static Cell * make_cell (Cell * cell) {
  Cell * r = malloc(1*sizeof(Cell));
  *r = *cell;
  return r;
}

static Cell * cell_cons (Cell * cell) {
  return make_cell(&(Cell){TCONS, .car_ = cell});
}
static Cell * cell_int    (int a)    { return make_cell(&(Cell){TINT,    .int_    = a}); }
static Cell * cell_symbol (char * a) {
  Cell * r = make_cell(&(Cell){TSYMBOL, .int_ = 0});
  r->symbol_ = malloc(sizeof(char) * (strlen(a) + 1) );
  strcpy(r->symbol_, a);
  return r;
}

static Cell * add_env (Cell * cell, Cell * env) {
  Cell * new = make_cell(cell);
  new->cdr_ = env;
  return new;
}
/* ---- ---- ---- ---- ---- ---- */

/* ---- ---- lex tools ---- ---- */
static void next (void) {
  c = fgetc(fp);
}
static int show_next (void) {
  int a = fgetc(fp);
  ungetc(a, fp);
  return a;
}
static Cell * read_int (int a) {
  int b = a - '0';
  while (isdigit(show_next())){
	next();
	b = b * 10 + (c - '0');
  }
  return cell_int(b);
}
static Cell * read_symbol (char a) {
  char buf[256];
  buf[0] = a;
  int s = 1;
  while (isalpha(show_next()) || strchr(symbols, show_next())) {
	next();
	buf[s++] = c;
  }
  buf[s] = '\0';
  return cell_symbol(buf);
  
}

/* ==== ==== ==== parser ==== ==== ==== */
static Cell * parse (void) {
  for(;;) {
	next();
	if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
	  continue;
	if (c == EOF)
	  return NULL;
	if (isdigit(c)) {
	  Cell * r = read_int(c);
	  r->cdr_  = parse();
	  return r;
	}
	if (isalpha(c) || strchr(symbols,c)) {
	  Cell * r = read_symbol(c);
	  r->cdr_  = parse();
	  return r;
	}
	if (c == '(') {
	  Cell * r = cell_cons(parse());
	  r->cdr_  = parse();
	  return r;
	}
	if (c == ')') {
	  return Nil;
	}
  }
}
/* ==== ==== ==== ====== ==== ==== ==== */

static void print_cell (Cell * cell) {
  if (cell == NULL)
	return;
  if (cell->type_ == TINT) {
	printf("%d ", cell->int_);
	if (cell->cdr_ != NULL || cell->cdr_ != Nil)
	  print_cell(cell->cdr_);
  }
  else if (cell->type_ == TSYMBOL) {
	printf("%s ", cell->symbol_);
	if (cell->cdr_ != NULL || cell->cdr_ != Nil)
	  print_cell(cell->cdr_);
  }
  else if (cell->type_ == TCONS){
	printf("(");
	print_cell(cell->car_);
	printf(") ");
	print_cell(cell->cdr_);
  }
}

/* ==== ==== ==== eval ==== ==== ==== */

/* ==== ==== ==== ==== ==== ==== ==== */

/* ==== ==== ==== main loop ==== ==== ==== */
int main (int argv, char* argc[])
{
  Cell * R;
  Cell * E = Nil;
  
  fp = fopen(argc[1], "r");
  R = parse();
  print_cell(E);

  return 0;
}
