#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#include<time.h>

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
	  return Nil;
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
  else if (cell->type_ == TTRUE) {
	printf("TRUE ");
	return;
  }
  else if (cell->type_ == TFALSE) {
	printf("FALSE ");
	return;
  }
  else if (cell->type_ == TNIL) {
	printf("nil ");
	return;
  }
}

/* ==== ==== ==== eval ==== ==== ==== */
static inline Cell * plus_eval  (Cell*, Cell*);
static inline Cell * minus_eval (Cell*, Cell*);
static inline Cell * time_eval  (Cell*, Cell*);
static inline Cell * divid_eval (Cell*, Cell*);
static inline Cell * great_eval (Cell*, Cell*);
static inline Cell * less_eval  (Cell*, Cell*);
static inline Cell * equal_eval (Cell*, Cell*);
static inline Cell * if_eval    (Cell*, Cell*);
static inline Cell * car_eval   (Cell*);
static inline Cell * cdr_eval   (Cell*);

static Cell * eval (Cell * cell, Cell * env) {
  Cell * e = cell;
  int etype = e->type_;
  switch(etype){
  case TNIL: case TTRUE: case TFALSE:
  case TINT:
	printf("simple\n");
	return e;
  case TCONS:{
	printf("cons\n");
	return eval(e->car_, env);
  }
  case TSYMBOL:{
	char * p = e->symbol_;
	if ( strcmp(p,"+") == 0 ) {
	  printf("'+' symbol\n");
	  return plus_eval(e->cdr_, env);
	}
	if ( strcmp(p,"-") == 0 ) {
	  printf("'-' symbol\n");
	  return minus_eval(e->cdr_, env);
	}
	if ( strcmp(p,"*") == 0 ) {
	  printf("'*' symbol\n");
	  return time_eval(e->cdr_, env);
	}
	if ( strcmp(p,"/") == 0 ) {
	  printf("'/' symbol\n");
	  return divid_eval(e->cdr_, env);
	}
	if ( strcmp(p,">") == 0 ) {
	  printf("'>' symbol\n");
	  return great_eval(e->cdr_, env);
	}
	if ( strcmp(p,"<") == 0 ) {
	  printf("'<' symbol\n");
	  return less_eval(e->cdr_, env);
	}
	if ( strcmp(p,"=") == 0 ) {
	  printf("'=' symbol\n");
	  return equal_eval(e->cdr_, env);
	}
	if ( strcmp(p,"if") == 0 ) {
	  printf("'if' symbol\n");
	  return if_eval(e->cdr_, env);
	}
	if ( strcmp(p,"car") == 0 ) {
	  printf("'car' symbol\n");
	  return car_eval(e->cdr_);
	}
	if ( strcmp(p,"cdr") == 0 ) {
	  printf("'cdr' symbol\n");
	  return cdr_eval(e->cdr_);
	}
  }
   
  }
  return Nil;
}

static inline Cell * plus_eval (Cell * cell, Cell * env) {
  int result = 0;
  Cell * p;
  for (p = cell; p != Nil; p = p->cdr_) {
	Cell * T = eval(p, env);
	if (T->type_ == TINT)
	  result += T->int_;
	else
	  printf("arg isnot 'TINT' for '+' symbols\n");
  }
  return cell_int(result);
}
static inline Cell * minus_eval (Cell * cell, Cell * env) {
  Cell * p = eval(cell,env);
  int result = p->int_;
  for (p = cell->cdr_; p != Nil; p = p->cdr_) {
	Cell * T = eval(p, env);
	if (T->type_ == TINT){
	  result -= T->int_;
	}
	else
	  printf("arg isnot 'TINT' for '-' symbols\n");
  }
  return cell_int(result);
}
static inline Cell * time_eval (Cell * cell, Cell * env) {
  int result = 1;
  Cell * p;
  for (p = cell; p != Nil; p = p->cdr_) {
	Cell * T = eval(p, env);
	if (T->type_ == TINT){
	  if (T->int_)
		result *= T->int_;
	  else
		return cell_int(0);
	}
	else
	  printf("arg isnot 'TINT' for '*' symbols\n");
  }
  return cell_int(result);
}
static inline Cell * divid_eval (Cell * cell, Cell * env) {
  Cell * p = eval(cell,env);
  int result = p->int_;
  for (p = cell->cdr_; p != Nil; p = p->cdr_){
	Cell * T = eval(p,env);
	if (T->type_ == TINT) {
	  if (T->int_)
		result /= T->int_;
	  else{
		printf("divided not have 0\n");
		return cell_int(0);
	  }
	}
	else
	  printf("arg isnot 'TINT' for '/' symbols\n");
  }
  return cell_int(result);
}
static inline Cell * great_eval (Cell * cell, Cell * env) {
  Cell * L = eval(cell,       env);
  Cell * R = eval(cell->cdr_, env);
  if (L->type_ == TINT) {
	if (L->int_ > R->int_)
	  return TRUE;
	else
	  return FALSE;
  }
  printf("greater error\n");
  return Nil;
}
static inline Cell * less_eval (Cell * cell, Cell * env) {
  Cell * L = eval(cell,       env);
  Cell * R = eval(cell->cdr_, env);
  if (L->type_ == TINT) {
	if (L->int_ < R->int_)
	  return TRUE;
	else
	  return FALSE;
  }
  printf("less error\n");
  return Nil;
}
static inline Cell * equal_eval (Cell * cell, Cell * env) {
  Cell * L = eval(cell,       env);
  Cell * R = eval(cell->cdr_, env);
  if (L->type_ == TINT) {
	if (L->int_ == R->int_)
	  return TRUE;
	else
	  return FALSE;
  }
  printf("equal error\n");
  return Nil;
}
static inline Cell * if_eval (Cell * cell, Cell * env) {
  Cell * p = eval(cell,env);
  if (p == TRUE)
	return eval (cell->cdr_, env);
  if (p == FALSE)
	return eval (cell->cdr_->cdr_, env);

  printf("if error\n");
  return Nil;
}
static inline Cell * car_eval (Cell * cell) {
  return cell->car_;
}
static inline Cell * cdr_eval (Cell * cell) {
  return cell->car_->cdr_;
}

/* ==== ==== ==== ==== ==== ==== ==== */

/* ==== ==== ==== main loop ==== ==== ==== */
int main (int argv, char* argc[])
{
  clock_t start , end;
  
  Cell * R;
  Cell * E = Nil;
  fp = fopen(argc[1], "r");

  start = clock();
  R = parse();
  end = clock();
  printf( "処理時間:%lu[ms]\n", end - start );
  
  print_cell(eval(R,E));

  return 0;
}
