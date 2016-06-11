PROG = main.c types.c parser.c
CFLAG=-Wall -ledit
COMPILE=gcc

.c.o:
	$(COMPILE) $(CFLAG) -c $?

lisp: ${PROG:.c=.o}
	$(COMPILE) $(CFLAG) -o $@ $^

.PHONY: run debug gdb valgrind

run:
	./list

debug: $(PROG)
	$(COMPILE) $(CFLAG) -g -O0 $^ -o DEBUG

gdb:
	gdb DEBUG

valgrind:
	valgrind --leak-check=full ./DEBUG

clean:
	rm -f lisp ${PROG:.c=.o}
