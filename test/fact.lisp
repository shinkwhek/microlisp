(define (fact n)
  (if (= n 0) 1 (* n (fact (- n 1)))))

(define r (fact 10))

(print r)
