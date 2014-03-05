CC=clang
CFLAGS=-g
LDFLAGS=-lreadline

all: repl test

repl: cache.o repl.c

test: cache.o test.c

cache.o: cache.c

cache.so: cache.c
	$(CC) -fPIC -shared cache.c -o cache.so

clean:
	rm -f repl repl.o cache.o cache.so test test.o
