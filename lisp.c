#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>

FILE * fp;
int c;

enum {
  TNIL = 0,
  TTRUE,
  TFALSE,
  TSYMBOL,
  TCONS,
  TINT
};

typedef struct cell_s {
  int type_;
  union {
	int int_;
	char * symbol_;
	struct cell_s * car_;
  };
  struct cell_s * cdr_;
} Cell;

static Cell * make_cell (Cell * cell) {
  Cell * r = malloc(1*sizeof(Cell));
  *r = *cell;
  return r;
}

/* ---- ---- make cell ---- ---- */
static Cell * cell_cons (Cell * cell) {
  return make_cell(&(Cell){TCONS, .car_ = cell});
}
static Cell * cell_int    (int a)    { return make_cell(&(Cell){TINT,    .int_    = a}); }
static Cell * cell_symbol (char * a) { return make_cell(&(Cell){TSYMBOL, .symbol_ = a}); }
/* ---- ---- ---- ---- ---- ---- */

static Cell * Nil   = &(Cell){ TNIL,   .int_ = 0 };
static Cell * TRUE  = &(Cell){ TTRUE,  .int_ = 1 };
static Cell * FALSE = &(Cell){ TFALSE, .int_ = 0 };

/* ---- ---- lex tools ---- ---- */
static void next (void) {
  c = fgetc(fp);
}
static int show_next (void) {
  int a = fgetc(fp);
  ungetc(a, fp);
  return a;
}
static int get_int (int a) {
  int b = a - '0';
  while (isdigit(show_next())){
	next();
	b = b * 10 + (c - '0');
  }
  return b;
}
static Cell * read_symbol (char a) {
  char buf[256];
  buf[0] = a;
  int s = 1;
  while (isalpha(show_next())) {
	next();
	buf[s++] = c;
  }
  buf[s] = '\0';
  return cell_symbol(buf);
  
}

static Cell * parse (void) {
  for(;;) {
	next();
	if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
	  continue;
	if (c == EOF)
	  return NULL;
	if (isdigit(c)) {
	  Cell * r = cell_int(get_int(c));
	  r->cdr_  = parse();
	  return r;
	}
	if (isalpha(c)) {
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

static void print_cell (Cell * cell) {
  if (cell == NULL)
	return;
  if (cell->type_ == TINT) {
	printf("%d ", cell->int_);
	if (cell->cdr_ != NULL || cell->cdr_ != Nil)
	  print_cell(cell->cdr_);
  }
  else if (cell->type_ == TSYMBOL)
	printf("%d ", cell->type_);
  else if (cell->type_ == TCONS){
	printf("(");
	print_cell(cell->car_);
	printf(") ");
	print_cell(cell->cdr_);
  }
}

int main (int argv, char* argc[])
{
  Cell * E;
  fp = fopen(argc[1], "r");
  E = parse();
  
  print_cell(E);

  return 0;
}
