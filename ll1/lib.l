(define penv (lambda () (print-list (env))))
(define ? (lambda (a b c) (if a b c)))

(define def define)
(define ? if)

(define make-adder (lambda (a) (lambda (b) (+ a b))))
(define 5adder (make-adder 5))


(penv)
