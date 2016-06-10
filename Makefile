PROG = main type
STET = -Wall -ledit
COMPILE = gcc

OUTF = lisp

$(OUTF): $(PROG).o
	$(COMPILE) $(STET) -o $@ $^

$(PROG).o: $(PROG).c
	$(COMPILE) $(STET) -c $?
