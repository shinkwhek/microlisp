(define f (lambda (a) (+ 1 a)))

(define g (lambda (a) (* 3 a)))

(define r1 (f (g 1)))
(define r2 (g (f 1)))

(print (+ r1 r2))

