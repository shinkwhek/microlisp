(def f (lambda (x) (+ x x)))
(def g (lambda (x y) (if (< x y) (* x y) (+ x y))))

(g (f 4) (f 5))
