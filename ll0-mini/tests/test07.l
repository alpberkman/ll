(define square (lambda (x) (* x x)))
(square 3)
(square 4)
((lambda (x) (- x 2)) 7)
(define make-adder (lambda (x) (lambda (y) (+ x y))))
(define add-two (make-adder 2))
(add-two 5)
