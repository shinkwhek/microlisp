PROGRAM = main
COMPILE = gcc
OUTPUT = avrlisp

.c.o: ${PROGRAM}.c
	$(COMPILE) -c $?

avrlisp: ${PROGRAM}.o
	$(COMPILE) -o $@ $^


clean:
	rm -f $(PROGRAM).o $(OUTPUT)
