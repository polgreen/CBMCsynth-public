(set-logic LIA)
(declare-fun x () Int)
(declare-fun y () Int)

(assert (= (+ 1 (+ x y)) y))
(assert (= (+ x y) (+ 1 2)))

(check-sat)


