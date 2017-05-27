(define f (lambda (x) (+ x x)))
(define g (lambda (x y) (* x y)))

(define x (f 3))
(define y (g 3 4))

(print x)
(print y)
