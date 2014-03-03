CC=clang
CFLAGS=-g
LDFLAGS=-lreadline

all: test

test: cache.o test.c

cache.o: cache.c

clean:
	rm -f test test.o cache.o
