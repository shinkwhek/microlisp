# AVRLISP

[![MIT License](http://img.shields.io/badge/license-MIT-blue.svg?style=flat)](LICENSE)

Small LISP on AVR.

## Documentation

This is a Lisp interpreter, which is so small, on AVR micro computer.

### Example

```lisp
> 1
; <NUMBER> = 1.

> +
; <PRIMITIVE>.

> (+ 1 3 (/ 4 2) (* 7 (- 9 5)))
; <NUMBER> = 34.

> (car (1 2))
; <NUMBER> = 1.

> (cdr (1 2))
; <NUMBER> = 2.

> (cons 1 2)
; (<SEXPR> . <SEXPR>).

> (> 1 2)
; <BOOL> = False.

> (= 1 1)
; <BOOL> = True.

> (if (< 1 2) (+ 1 1) (- 1 1))
; <NUMBER> = 2.

> (define x 1)
; <NUMBER> = 1.

> x
; <NUMBER> = 1.

> (define plus +)
; <PRIMITIVE>.

> (plus x x)
; <NUMBER> = 2.

> ((lambda (x) (+ x x)) 3)
; <NUMBER> = 6.

> (define f (lambda (x y) (* x y)))
; <CONS>.

> (f 3 4)
; <NUMBER> = 12.
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
