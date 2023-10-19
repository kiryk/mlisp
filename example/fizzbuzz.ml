(def i 1)
(while (<= i 100) (do
	(print (if (= (% i 15) 0) "fizzbuzz"
	       (if (= (% i 5)  0) "buzz"
	       (if (= (% i 3)  0) "fizz"
	                          i))))
	(set i (+ i 1))))
