# AVRLISP

[![MIT License](http://img.shields.io/badge/license-MIT-blue.svg?style=flat)](LICENSE)

Small LISP on AVR.

## Documentation

This is a Lisp interpreter, which is so small, on AVR micro computer.

Main file is "**lisp.c**". "**main.c**" is a first project file, but its garbage now.

### How to be expressive of data structure of lisp

#### S-Expression

code :arrow_down:

```c
typedef struct SExpr {
  int type;
  void *car;
  void *cdr;
} SExpr;
```

```
type := <INT> // type of SExpr
car  := <INT> | <STRING> | <SEXPR>
cdr  := <NIL> | <SEXPR>
```

#### Environment

code :arrow_down:

```c
typedef struct SExpr *primFUN(struct Env *_env , SExpr *_args);

typedef struct Env {
  int type;
  primFUN *fn;
  void *car;
  void *cdr;
  struct Env *next;
} Env;
```

```
type := <INT>
fn   := <FUNCTION>           // included primitive function
car  := <STRING>             // Name of symbol or function
cdr  := <INT> | <STRING>     // Symbol value or string
next := <ENV>
```

...

## Build

```shell
$ make
```
```shell
$ ./lisp
`````

## Write

## Author

Shin KAWAHARA
