PROG = lisp.c

CFLAGS = -g -Wall

COMPILE=gcc

.c.o:
	$(COMPILE) $(CFLAGS) -c $?

lisp: ${PROG:.c=.o}
	$(COMPILE) $(CFLAGS) -o $@ $^

.PHONY: run gdb valgrind

run:
	./lisp

gdb: $(PROG)
	$(COMPILE) $(CFLAGS) -O0 $^ -o DEBUG
	sudo gdb DEBUG

valgrind:
	$(COMPILE) $(CFLAGS) -O0 $^ -o DEBUG
	valgrind --leak-check=full ./DEBUG

clean:
	rm -f lisp ${PROG:.c=.o}
