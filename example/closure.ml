(def alternate (fn (a b)
	(def which nil)
	(fn ()
		(set which (not which))
		(if which a b))))

(def cl (alternate "flip" "flop"))

(print (cl))
(print (cl))
(print (cl))
(print (cl))
