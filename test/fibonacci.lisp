(define fib (lambda (n)
			  (if (< n 2) n
				(+ (fib (- n 1)) (fib (- n 2))))))

(define result (fib 10))

(print result)
