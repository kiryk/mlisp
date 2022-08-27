CC = cc
LD = ld

FLAGS = -Wall -Werror

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
	$(CC) -o $@ -c $<

clean:
	rm mlisp $(OBJS)
