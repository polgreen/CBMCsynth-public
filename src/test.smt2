(set-logic LIA)
(declare-fun x () Int)
(declare-fun y () Int)
(assert (= 16 (+ 1 x)))
(assert (= 13 y))
(check-sat)


