(def primes (list))
(def n 0)
(def p 2)
(def i 0)

(print "the number of primes to be found:")
(set n (read))

(while (< (len primes) n) (do
	(set i 0)
	(while (and (< i (len primes))
	            (!= 0 (% p (list-get primes i))))
	  (set i (+ i 1)))
	(if (= i (len primes)) (do
		(print p)
		(set (list-get primes i) p)))
	(set p (+ p 1))))
