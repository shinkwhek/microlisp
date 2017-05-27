(define (abs a)
  (if (< a 0.0) (- 0.0 a) a))

(define (square x) (* x x))

(define (average a b)
  (/ (+ a b) 2.0))

(define (improve a b)
  (average a (/ b a)))

(define (good? guess x)
  (< (abs (- (square guess) x)) 0.001))

(define (sqrt-iter guess x)
  (if (good? guess x)
    guess
    (sqrt-iter (improve guess x)
	       x)))

(define (newton-sqrt n)
  (sqrt-iter 1.0 n))

(print (newton-sqrt 5.0))
