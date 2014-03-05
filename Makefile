CC=clang
CFLAGS=-g
LDFLAGS=-lreadline

all: test

test: cache.o test.c

cache.o: cache.c

cache.so: cache.c
	$(CC) -fPIC -shared cache.c -o cache.so

clean:
	rm -f cache.o cache.so test test.o
