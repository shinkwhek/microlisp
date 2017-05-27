;; 最大公約数
(define (my_gcd a b)
  (if (= (mod a b) 0)
    b (my_gcd b (mod a b))))

;; 最小公倍数
(define (my_lcd a b)
  (/ (* b a) (my_gcd a b)))

(print (/ (my_lcd 7 31) (my_gcd 35 21)))
