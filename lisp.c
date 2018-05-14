#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>

FILE * fp;
int c;		// current
int flag_list;

/* ==== ==== ==== type ==== ==== ==== */
enum {
  TNIL = 0,
  TUNIT,
  TENV,
  TTRUE,
  TFALSE,
  TSYMBOL,
  TCONS,
  TLIST,
  TFUN,
  TINT,
  TREAL
};

struct cell_s;

typedef struct cell_s {
  int type_;
  union {
    // data
    int int_;
    float real_;
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
  Cell * r;
  if ((r = malloc(1 * sizeof(Cell))) == NULL) {
    printf("[error]: malloc\n");
    exit(-1);
  }
  *r = *cell;
  return r;
}

static Cell * cell_cons (Cell * cell) { return make_cell(&(Cell){ TCONS, .car_ = cell }); }
static Cell * cell_list (Cell * cell) { return make_cell(&(Cell){ TLIST, .car_ = cell }); }
static Cell * cell_int  (int a)       { return make_cell(&(Cell){ TINT, .int_=a });       }
static Cell * cell_real (float a)     { return make_cell(&(Cell){ TREAL, .real_=a});      }
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

#define skip \
  do { next; } while(c != '\n')

static inline int show_next (void) {
  int a = fgetc(fp);
  ungetc(a, fp);
  return a;
}

static Cell * parse_num (int a, int neg) {
  int int_or_real = 0;
  int b    = a - '0';          // int
  float br = (float)(a - '0'); // real
  while (isdigit(show_next())){
    next;
    b  = b  * 10 + (int)(c - '0');   // int
    br = br * 10 + (float)(c - '0'); // real
  }
  if (show_next() == '.') {
    int_or_real = 1;
    next;
    int k = 0;
    while(isdigit(show_next())) {
      next;
      k++;
      br = br * 10.0 + (float)(c - '0'); // real
    }
    for(int p = 0; p < k; p++) {
      br = br / 10.0;
    }
  }
  if (neg == 1) {
    if (int_or_real == 0)
      b = -b;
    else
      br = -br;
  }
  if (int_or_real == 0)
    return cell_int(b);
  else
    return cell_real(br);
}

static Cell * parse_symbol (char a) {
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
    if (c == ';') { // comment
      skip;
      continue;
    }
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
      continue;
    if (c == EOF)
      return Nil;
    if (c == '-' && isdigit(show_next())) {
      next;
      Cell * r = parse_num(c, 1);
      r->cdr_ = parse();
      return r;
    }
    if (isdigit(c)) {
      Cell * r = parse_num(c, 0);
      r->cdr_  = parse();
      return r;
    }
    if (isalpha(c) || strchr(symbols,c)) {
      Cell * r = parse_symbol(c);
      r->cdr_  = parse();
      return r;
    }
    if (c == '\'' && show_next() == '(') {
      next;
      flag_list = 1;
      Cell * r = cell_list(parse());
      flag_list = 0;
      r->cdr_ = parse();
      return r;
    }
    if (c == '(') {
      Cell * r;
      if (flag_list == 1) r = cell_list(parse());
      else                r = cell_cons(parse());
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
static inline Cell * mod_eval    (Cell*, Cell**);
static inline Cell * great_eval  (Cell*, Cell**);
static inline Cell * less_eval   (Cell*, Cell**);
static inline Cell * equal_eval  (Cell*, Cell**);
static inline Cell * and_eval    (Cell*, Cell**);
static inline Cell * or_eval     (Cell*, Cell**);
static inline Cell * if_eval     (Cell*, Cell**);
static inline Cell * car_eval    (Cell*, Cell**);
static inline Cell * cdr_eval    (Cell*, Cell**);
static inline Cell * list_eval   (Cell*, Cell**);
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
      primitive(mod,       mod);
      primitive(great,       >);
      primitive(less,        <);
      primitive(equal,       =);
      primitive(and,       and);
      primitive(or,         or);
      primitive(if,         if);
      primitive(car,       car);
      primitive(cdr,       cdr);
      primitive(list,     list);
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
    case TNIL: case TTRUE: case TFALSE: case TINT: case TREAL: case TFUN: case TLIST:
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
  int   result   = 0;
  float result_r = 0;
  int type;
  Cell * Tp = eval(cell, env);
  if (Tp->type_ == TINT)   // setting type && culc
    result += Tp->int_;
  else if (Tp->type_ == TREAL)
    result_r += Tp->real_;
  type = Tp->type_;
  for (Cell * p = cell->cdr_; p != Nil; p = p->cdr_) { // culc
    Cell * T = eval(p, env);
    if (T->type_ == TINT && type == TINT)
      result += T->int_;
    else if (T->type_ == TREAL && type == TREAL)
      result_r += T->real_;
    else
      perror("type error in '+'.");
  }
  if (type == TINT)
    return cell_int(result);
  else
    return cell_real(result_r);
}

// (- _ ...)
static inline Cell * minus_eval (Cell * cell, Cell ** env) {
  Cell * p = eval(cell,env);
  int   result;
  float result_r;
  if (p->type_ == TINT) // check type
    result = p->int_;
  else if (p->type_ == TREAL)
    result_r = p->real_;
  int type = p->type_;
  for (p = cell->cdr_; p != Nil; p = p->cdr_) { // culc
    Cell * T = eval(p, env);
    if (T->type_ == TINT && type == TINT)
      result -= T->int_;
    else if (T->type_ == TREAL && type == TREAL)
      result_r -= T->real_;
    else
      perror("type error in '-'.");
  }
  if (type == TINT)
    return cell_int(result);
  else
    return cell_real(result_r);
}

// (* _ ...)
static inline Cell * time_eval (Cell * cell, Cell ** env) {
  int   result = 1;
  float result_r = 1.0;
  Cell * Tp = eval(cell, env);
  if (Tp->type_ == TINT)
    result *= Tp->int_;
  else if (Tp->type_ == TREAL)
    result_r *= Tp->real_;
  int type = Tp->type_;
  for (Cell * p = cell->cdr_; p != Nil; p = p->cdr_) {
    Cell * T = eval(p, env);
    if (T->type_ == TINT && type == TINT){
      if (T->int_ != 0)
        result *= T->int_;
      else
        return cell_int(0);
    }
    else if (T->type_ == TREAL && type == TREAL) {
      if (T->real_ != 0)
        result_r *= T->real_;
      else
        return cell_real(0);
    }
    else
      perror("type error in '*'.");
  }
  if (type == TINT)
    return cell_int(result);
  else
    return cell_real(result_r);
}

// (/ _ ...)
static inline Cell * divid_eval (Cell * cell, Cell ** env) {
  Cell * p = eval(cell,env);
  int type = p->type_;
  int result;
  float result_r;
  if (type == TINT)
    result = p->int_;
  else if (type == TREAL)
    result_r = p->real_;
  for (p = cell->cdr_; p != Nil; p = p->cdr_){
    Cell * T = eval(p,env);
    if (T->type_ == TINT && type == TINT) {
      if (T->int_ != 0)
        result /= T->int_;
      else{
        perror("divided not have 0");
      }
    }else if (T->type_ == TREAL && type == TREAL) {
      if (T->real_ != 0.0)
        result_r /= T->real_;
      else{
        perror("divided not have 0");
      }
    }
    else
      printf("type error in '/'.");
  }
  if (type == TINT)
    return cell_int(result);
  else
    return cell_real(result_r);
}

static inline Cell * mod_eval (Cell * cell, Cell ** env) {
  Cell * L = eval(cell,       env);
  Cell * R = eval(cell->cdr_, env);
  int type = L->type_;
  if (type == TINT && R->type_ == TINT)
    return cell_int(L->int_ % R->int_);
  else
    perror("type errpr in 'mod'.");
  return Nil;
}

// (> _ _)
static inline Cell * great_eval (Cell * cell, Cell ** env) {
  Cell * L = eval(cell,       env);
  Cell * R = eval(cell->cdr_, env);
  if (L->type_ == TINT && R->type_ == TINT)
    return (L->int_ > R->int_) ? TRUE : FALSE;
  else if (L->type_ == TREAL && R->type_ == TREAL)
    return (L->real_ > R->real_) ? TRUE : FALSE;
  perror("type error in '>'");
  return Nil;
}

// (< _ _)
static inline Cell * less_eval (Cell * cell, Cell ** env) {
  Cell * L = eval(cell,       env);
  Cell * R = eval(cell->cdr_, env);
  if (L->type_ == TINT && R->type_ == TINT)
    return (L->int_ < R->int_) ? TRUE : FALSE;
  else if (L->type_ == TREAL && R->type_ ==TREAL)
    return (L->real_ < R->real_) ? TRUE : FALSE;
  perror("type error in '<'");
  return Nil;
}

// (= _ _)
static inline Cell * equal_eval (Cell * cell, Cell ** env) {
  Cell * L = eval(cell,       env);
  Cell * R = eval(cell->cdr_, env);
  if (L->type_ == TINT && R->type_ == TINT)
    return (L->int_ == R->int_) ? TRUE : FALSE;
  else if (L->type_ == TREAL && R->type_ == TREAL)
    return (L->real_ == R->real_) ? TRUE : FALSE;
  printf("equal error\n");
  return Nil;
}

// (and _ ...)
static inline Cell * and_eval (Cell * cell, Cell ** env) {
  for (Cell * p = cell; p != Nil; p=p->cdr_) {
    Cell * T = eval(p, env);
    if (T == FALSE)
      return FALSE;
  }
  return TRUE;
}

// (or _ ...)
static inline Cell * or_eval (Cell * cell, Cell ** env) {
  for (Cell * p = cell; p != Nil; p=p->cdr_) {
    Cell * T = eval(p, env);
    if (T == TRUE)
      return TRUE;
  }
  return FALSE;
}

// (if _ _ _)
static inline Cell * if_eval (Cell * cell, Cell ** env) {
  Cell * p = eval(cell,env);
  if (p == TRUE)  return eval (cell->cdr_, env);
  if (p == FALSE) return eval (cell->cdr_->cdr_, env);
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

// (lambda (_ ...) _)
static inline Cell * lambda_eval (Cell * cell, Cell ** env) {
  return make_cell(&(Cell){ TFUN, .car_=cell->car_, .cdr_=cell->cdr_ });
}

// (list ...)
static inline Cell * list_eval (Cell * cell, Cell ** env) {
  Cell * r = eval(cell,env);
  Cell * p = r;
  for (Cell * t = cell->cdr_; t != Nil; t = t->cdr_) {
    r->cdr_ = eval(t, env);
    r = r->cdr_;
  }
  return cell_list(p);
}

// (define _ _)
static inline Cell * def_eval (Cell * cell, Cell ** env) {
  Cell * new_env = NULL;
  if (cell->type_ != TCONS) { // (define var body)
    new_env = cell_cons(cell);
    new_env->car_->cdr_ = eval(cell->cdr_, env);
    new_env->cdr_ = *env;
    *env = new_env;
  } else { // (define (fname lvar) body)
    Cell * fname = cell->car_;
    Cell * lvars  = cell->car_->cdr_;
    Cell * body  = cell->cdr_;
    fname->cdr_ = make_cell(&(Cell){ TFUN, .car_=lvars, .cdr_=body });
    new_env = cell_cons(fname);
    new_env->cdr_ = *env;
    *env = new_env;
  }
  return new_env;
}

// (print _)
static inline void print_eval_iter (Cell * cell, Cell ** env) {
  switch(cell->type_) {
    case TNIL:    printf("nil");                         break;
    case TTRUE:   printf("#t");                          break;
    case TFALSE:  printf("#f");                          break;
    case TINT:    printf("%d", cell->int_);              break;
    case TREAL:   printf("%f", cell->real_);             break;
    case TFUN:    printf("lambda function");             break;
    case TCONS:   print_eval_iter(eval(cell, env), env); break;
    case TLIST: {
                  printf("(");
                  for (Cell * r = cell->car_; r != Nil; r = r->cdr_) {
                    print_eval_iter(r, env);
                    if (r->cdr_ != Nil) printf(" ");
                  }
                  printf(")");
                  break;
                }
    case TSYMBOL:
                  print_eval_iter(find_symbol(cell, env), env);  break;
    case TENV:
                  printf("env");   break;
    default:
                  printf("print nothing");
  }
}
static inline Cell * print_eval (Cell * cell, Cell ** env) {
  print_eval_iter(cell, env);
  printf("\n");
  return make_cell(&(Cell){TUNIT});
}
/* ==== ==== ==== ==== ==== ==== ==== */

/* ==== ==== ==== main loop ==== ==== ==== */
static void file_read_mode(char* argv[], Cell* E) {
  fp = fopen(argv[1], "r");
  Cell * R = parse();
  do {
    eval(R, &E);
    R = R->cdr_;
  } while( R != Nil );
}

int main (int argc, char* argv[])
{
  Cell * E = Nil;
  flag_list = 0;

  if (argc <= 1) {
    perror("no input file.");
    exit(1);
  }

  if (argv[1]){
    if (strcmp( ".scm" ,strstr(argv[1],".") ) != 0)
      perror("file format is not .scm");
    else {
      file_read_mode(argv, E);
   }
  }

  return 0;
}
