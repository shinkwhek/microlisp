(define (ackermann m n)
  (if (= m 0) (+ n 1)
    (if (= n 0) (ackermann (- m 1) 1)
      (ackermann (- m 1) (ackermann m (- n 1))))))


(print (ackermann 3 1))
