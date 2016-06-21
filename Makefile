PROG = main.c parser.c

CFLAGS = -g -Wall -ledit

COMPILE=gcc

.c.o:
	$(COMPILE) $(CFLAGS) -c $?

lisp: ${PROG:.c=.o}
	$(COMPILE) $(CFLAGS) -o $@ $^

.PHONY: run debug gdb valgrind

run:
	./lisp

debug: $(PROG)
	$(COMPILE) $(CFLAGS) -O0 $^ -o DEBUG

gdb:
	gdb DEBUG
	rm -f DEBUG

valgrind:
	valgrind --leak-check=full ./DEBUG
	rm -f DEBUG

clean:
	rm -f lisp ${PROG:.c=.o}
