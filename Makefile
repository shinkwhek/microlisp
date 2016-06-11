PROG = main.c types.c

lisp: $(PROG)
	gcc -Wall -o $@ -ledit $^

clean:
	rm -f lisp
