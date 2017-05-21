#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>

FILE * fp;
int c;		// current

/* ==== ==== ==== type ==== ==== ==== */
enum {
  TNIL = 0,
  TUNIT,
  TENV,
  TTRUE,
  TFALSE,
  TSYMBOL,
  TCONS,
  TFUN,
  TINT
};

struct cell_s;

typedef struct cell_s {
  int type_;
  union {
	// data
	int int_;
	char * symbol_;
	struct cell_s * car_;
  };
  struct cell_s * cdr_;
} Cell;

static Cell * Nil   = &(Cell){ TNIL,   .int_ = 0 };
static Cell * TRUE  = &(Cell){ TTRUE,  .int_ = 1 };
static Cell * FALSE = &(Cell){ TFALSE, .int_ = 0 };

static const char symbols[] = "+-*/!?=<>_:\\%#~&";
/* ==== ==== ==== ==== ==== ==== ==== */

/* ---- ---- make cell ---- ---- */
static Cell * make_cell (Cell * cell) {
  Cell * r = malloc( 1 * sizeof(Cell) );
  *r = *cell;
  return r;
}

static Cell * cell_cons (Cell * cell) {
  return make_cell(&(Cell){ TCONS, .car_ = cell });
}
static Cell * cell_int (int a) {
  return make_cell(&(Cell){ TINT, .int_=a });
}
static Cell * cell_symbol (char * a) {
  Cell * r = make_cell(&(Cell){ TSYMBOL, .int_ = 0 });
  r->symbol_ = malloc( sizeof(char) * (strlen(a)+1) );
  strcpy(r->symbol_, a);
  return r;
}
/* ---- ---- ---- ---- ---- ---- */

/* ---- ---- lex tools ---- ---- */
#define next \
  do { c = fgetc(fp); } while(0)

static int show_next (void) {
  int a = fgetc(fp);
  ungetc(a, fp);
  return a;
}
static Cell * read_int (int a) {
  int b = a - '0';
  while (isdigit(show_next())){
	next;
	b = b * 10 + (c - '0');
  }
  return cell_int(b);
}
static Cell * read_symbol (char a) {
  char buf[256];
  buf[0] = a;
  int s = 1;
  while (isalpha(show_next()) || isdigit(show_next()) || strchr(symbols, show_next())) {
	next;
	buf[s++] = c;
  }
  buf[s] = '\0';
  return cell_symbol(buf);
}

/* ==== ==== ==== parser ==== ==== ==== */
static Cell * parse (void) {
  for(;;) {
	next;
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

/* ==== ==== ==== eval ==== ==== ==== */
static inline Cell * plus_eval   (Cell*, Cell**);
static inline Cell * minus_eval  (Cell*, Cell**);
static inline Cell * time_eval   (Cell*, Cell**);
static inline Cell * divid_eval  (Cell*, Cell**);
static inline Cell * great_eval  (Cell*, Cell**);
static inline Cell * less_eval   (Cell*, Cell**);
static inline Cell * equal_eval  (Cell*, Cell**);
static inline Cell * if_eval     (Cell*, Cell**);
static inline Cell * car_eval    (Cell*, Cell**);
static inline Cell * cdr_eval    (Cell*, Cell**);
static inline Cell * def_eval    (Cell*, Cell**);
static inline Cell * lambda_eval (Cell*, Cell**);
static inline Cell * print_eval  (Cell*, Cell**);
static Cell * eval (Cell*, Cell**);

static Cell * set_lambda_args (Cell * name_l_, Cell * v_l_, Cell ** env ) {
  Cell * local_env = *env;
  Cell * name_l;
  Cell * v_l;
  for (name_l = name_l_, v_l = v_l_; name_l!=Nil && v_l!=Nil; name_l = name_l->cdr_, v_l = v_l->cdr_) {
	Cell * A       = make_cell(&(Cell){ TCONS, .car_=name_l->car_, .cdr_=eval(v_l,env) });
	Cell * new_env = make_cell(&(Cell){ TCONS, .car_=A,            .cdr_=local_env });
	local_env = new_env;
  }
  return local_env;
}

#define primitive(n,s) \
  do{if (strcmp(cell->symbol_,#s)==0) return n##_eval(args, env);}while(0)

static Cell * apply (Cell * cell, Cell * args, Cell ** env) {
  switch(cell->type_){
  case TSYMBOL:
	primitive(plus,        +);
	primitive(minus,       -);
	primitive(time,        *);
	primitive(divid,       /);
	primitive(great,       >);
	primitive(less,        <);
	primitive(equal,       =);
	primitive(if,         if);
	primitive(car,       car);
	primitive(cdr,       cdr);
	primitive(def,    define);
	primitive(lambda, lambda);
	primitive(print,   print);
	break;
  case TFUN:{
	Cell * local_env = *env;
	local_env = set_lambda_args(cell->car_, args, env);
	return eval(cell->cdr_, &local_env);
  }
  }
  return Nil;
}

static Cell * find_symbol (Cell * cell, Cell ** env) {
  for (Cell * E = *env; E != Nil; E = E->cdr_ ) {
	if (strcmp(cell->symbol_,E->car_->symbol_)==0) {
	  return E->car_->cdr_;
	}
  }
  return cell;
}

static Cell * eval (Cell * cell, Cell ** env) {
  switch(cell->type_){
  case TUNIT: case TENV:
	break;
  case TNIL: case TTRUE: case TFALSE: case TINT: case TFUN:
	return cell;
  case TCONS:
	return apply( eval(cell->car_,env), cell->car_->cdr_, env);
  case TSYMBOL:
	return find_symbol(cell, env);
  }

  return eval(cell->cdr_, env);
}

// (+ _ ...)
static inline Cell * plus_eval (Cell * cell, Cell ** env) {
  int result = 0;
  for (Cell * p = cell; p != Nil; p = p->cdr_) {
	Cell * T = eval(p, env);
	if (T->type_ == TINT)
	  result += T->int_;
	else
	  printf("arg isnot 'TINT' for '+' symbols\n");
  }
  return cell_int(result);
}
// (- _ ...)
static inline Cell * minus_eval (Cell * cell, Cell ** env) {
  Cell * p = eval(cell,env);
  int result = p->int_;
  for (p = cell->cdr_; p != Nil; p = p->cdr_) {
	Cell * T = eval(p, env);
	if (T->type_ == TINT)
	  result -= T->int_;
	else
	  printf("arg isnot 'TINT' for '-' symbols\n");
  }
  return cell_int(result);
}
// (* _ ...)
static inline Cell * time_eval (Cell * cell, Cell ** env) {
  int result = 1;
  for (Cell * p = cell; p != Nil; p = p->cdr_) {
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
// (/ _ ...)
static inline Cell * divid_eval (Cell * cell, Cell ** env) {
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
// (> _ _)
static inline Cell * great_eval (Cell * cell, Cell ** env) {
  Cell * L = eval(cell,       env);
  Cell * R = eval(cell->cdr_, env);
  if (L->type_ == TINT)
	return (L->int_ > R->int_) ? TRUE : FALSE;
  printf("greater error\n");
  return Nil;
}
// (< _ _)
static inline Cell * less_eval (Cell * cell, Cell ** env) {
  Cell * L = eval(cell,       env);
  Cell * R = eval(cell->cdr_, env);
  if (L->type_ == TINT)
	return (L->int_ < R->int_) ? TRUE : FALSE;
  printf("less error\n");
  return Nil;
}
// (= _ _)
static inline Cell * equal_eval (Cell * cell, Cell ** env) {
  Cell * L = eval(cell,       env);
  Cell * R = eval(cell->cdr_, env);
  if (L->type_ == TINT)
	return (L->int_ == R->int_) ? TRUE : FALSE;
  printf("equal error\n");
  return Nil;
}
// (if _ _ _)
static inline Cell * if_eval (Cell * cell, Cell ** env) {
  Cell * p = eval(cell,env);
  if (p == TRUE)
	return eval (cell->cdr_, env);
  if (p == FALSE)
	return eval (cell->cdr_->cdr_, env);

  printf("if error\n");
  return Nil;
}
// (car _)
static inline Cell * car_eval (Cell * cell, Cell ** env) {
  return cell->car_;
}
// (cdr _)
static inline Cell * cdr_eval (Cell * cell, Cell ** env) {
  return cell->car_->cdr_;
}
// (define _ _)
static inline Cell * def_eval (Cell * cell, Cell ** env) {
  Cell * new_env = cell_cons(cell);
  new_env->car_->cdr_ = eval(cell->cdr_, env);
  new_env->cdr_ = *env;
  *env = new_env;
  return new_env;
}
// (lambda (_ ...) _)
static inline Cell * lambda_eval (Cell * cell, Cell ** env) {
  return make_cell(&(Cell){ TFUN, .car_=cell->car_,.cdr_=cell->cdr_ });
}
// (print _)
static inline Cell * print_eval (Cell * cell, Cell ** env) {
  switch(cell->type_) {
  case TNIL:
	printf("nil");
	break;
  case TTRUE:
	printf("#t");
	break;
  case TFALSE:
	printf("#f");
	break;
  case TINT:
	printf("%d", cell->int_);
	break;
  case TFUN:
	printf("lambda function");
	break;
  case TCONS:
	printf("cons cell");
	break;
  case TSYMBOL:
	print_eval(find_symbol(cell, env), env);
	break;
  case TENV:
	printf("env");
	break;
  default:
	printf("nothing");
  }
  if (cell->type_ != TSYMBOL)
	printf("\n");
  return make_cell(&(Cell){TUNIT});
}
/* ==== ==== ==== ==== ==== ==== ==== */

/* ==== ==== ==== main loop ==== ==== ==== */
int main (int argv, char* argc[])
{
  Cell * E = Nil;

  if (argv <= 1) {
	perror("no input file.");
	exit(1);
  }
  
  if (argc[1]){
	if (strcmp( ".lisp" ,strstr(argc[1],".") ) != 0)
	  perror("file format is not .lisp");
	else {
	  fp = fopen(argc[1], "r");
	  Cell * R = parse();
	  do {
		eval(R, &E);
		R = R->cdr_;
	  } while( R != Nil );
	}
  }
  
  return 0;
}
