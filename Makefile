
CC			= cc
LISP		= tinylisp
FLAG		= -W -Wall -O0

FILE		= lisp.c

BIN 		= bin

RM			= rm

all: $(BIN)/$(LISP)

$(BIN)/$(LISP): $(FILE)
	$(CC) $(FLAG) -o $@ $^

debug: $(FILE)
	$(CC) $(FLAG) -g $^ -o $(BIN)/debug
	sudo gdb $(BIN)/debug

clean:
	$(RM) -rf *.o $(BIN)/$(LISP) $(BIN)/debug*
