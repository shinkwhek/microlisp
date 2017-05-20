
CC	= gcc
LISP	= lisp
FLAG	= -Wall -O0

FILE	= lisp.c

BIN 	= bin

RM	= rm -rf

all: $(BIN)/$(LISP)

$(BIN)/$(LISP): $(FILE)
	$(CC) $(FLAG) -o $@ $^

debug: $(FILE)
	$(CC) $(FLAG) -g $^ -o $(BIN)/debug
	sudo gdb $(BIN)/debug

test: all test/*.lisp
	prove -j 4 test/*.lisp

clean:
	$(RM) *.o $(BIN)/$(LISP) $(BIN)/debug*

.PHONY: all $(BIN)/$(LISP) debug test clean
