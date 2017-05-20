(define f (lambda (x)
	    (if (= x 0) 1 (+ x (f (- x 1))))))

(define result (f 3))

(print result)
