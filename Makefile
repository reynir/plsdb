CC=clang
CFLAGS=-g
LDFLAGS=-lreadline

all: examples/repl examples/test

examples/repl: cache.o examples/repl.c

examples/test: cache.o examples/test.c

cache.o: cache.c

cache.so: cache.c
	$(CC) -fPIC -shared cache.c -o cache.so

clean:
	rm -f cache.o cache.so
	rm -f examples/repl examples/test 
