# AVRLISP

[![MIT License](http://img.shields.io/badge/license-MIT-blue.svg?style=flat)](LICENSE)

Small LISP on AVR.

## Documentation

This is a Lisp interpreter, which is so small, on AVR micro computer.

### Example

```lisp
> 1
; <N> = 1.

> +
; <P>.

> (+ 1 3 (/ 4 2) (* 7 (- 9 5)))
; <NR> = 34.

> (a (1 2))
; <N> = 1.

> (d (1 2))
; <N> = 2.

> (s 1 2)
; (<SEXPR> . <SEXPR>).

> (> 1 2)
; <B> = F.

> (= 1 1)
; <B> = T.

> (i (< 1 2) (+ 1 1) (- 1 1))
; <N> = 2.

> (f x 1)
; <N> = 1.

> x
; <N> = 1.

> (f ps +)
; <P>.

> (ps x x)
; <N> = 2.

> ((\ (x) (+ x x)) 3)
; <N> = 6.

> (f a (\ (x y) (* x y)))
; <C>.

> (a 3 4)
; <N> = 12.
```

...

## Build

```shell
$ make
```

## Write

Write

```shell
$ make flash
```

Clean

```shell
$ make clean
```

## Author

Shin KAWAHARA
