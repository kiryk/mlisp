CC = cc

FLAGS = -Wall -Werror -ansi

OBJS = \
	debug.o \
	expr.o \
	main.o \
	map.o \
	mem.o \
	vector.o \

mlisp: $(OBJS)
	$(CC) $(FLAGS) -o mlisp $(OBJS)

%.o: %.c
	$(CC) $(FLAGS) -o $@ -c $<

clean:
	rm mlisp $(OBJS)
