(def double-op (fn (op n)
  (op n n)))

(def twice (fn (n)
  (double-op + n)))

(def squared (fn (n)
  (double-op * n)))

(print (twice 4) (squared 4)) ; prints '8 16'
