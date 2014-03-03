#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

typedef struct cache_node {
	int id;
	struct cache_node *next;
	size_t len;
} cache_node;

#define CACHE_SIZE ((size_t) 4 * 1024)

cache_node *cache_start = NULL;
cache_node *cache_last = NULL;

int gen_id(void)
{
	static int id = 0;
	return id++;
}

char *cache_find(int id)
{
	cache_node *curr = cache_start;
	if (curr->id == id) {
		return (char *) (curr + 1);
	}
	if (curr->next == NULL) {
		return NULL;
	}

	curr = curr->next;
	while (curr != NULL && curr != cache_start) {
		if (curr->id == id) {
			return (char *) (curr + 1);
		}
		curr = curr->next;
	}
	return NULL;
}

char *cache_add(int id, char *string, size_t len)
{
	assert( strlen(string) + 1 == len );
	/* return NULL if the string doesn't fit at all */
	if ( sizeof(cache_node) + len >= CACHE_SIZE ) {
		return NULL;
	}

	/* First addition */
	if (cache_last == NULL) {
		cache_last = cache_start;
		cache_last->id = id;
		cache_last->len = len;
		cache_last->next = NULL;
		memcpy( cache_last + 1, string, len );
		return (char *) (cache_last + 1);
	}

	cache_node *prev = cache_last;
	cache_node *next = prev->next;
	cache_node *curr = (cache_node *) (((char *) (prev + 1)) + prev->len);

	/* if string doesn't fit after curr */
	if ( (char *) (curr + 1) + len >= (char *) cache_start + CACHE_SIZE ) {
		curr = cache_start;
	}

	curr->id = id;
	curr->len = len;

	/* Update the next pointers */
	while ( curr < next &&
			/* Not enough space */
			(char *) (curr + 1) + len >= (char *) next ) {
		/* Free the next cache_node */
		next = prev->next = next->next;
	}

	memcpy( curr + 1, string, len );
	curr->next = next;
	cache_last = prev->next = curr;
	return (char *) (curr + 1);
}

void cache_init(int fd) {
	cache_start = mmap( NULL, CACHE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
	if (cache_start == MAP_FAILED) {
		printf( "MAP_FAILED!\n%s\n", strerror(errno) );
		exit(1);
	}
	cache_last = NULL;
}

int main(void)
{
	int fd = open("tmpfile",
			O_RDWR | O_CREAT | O_TRUNC,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	ftruncate(fd, CACHE_SIZE);
	assert(fd != -1);
	cache_init(fd);

	char s[100];
	for (int i = 0; i < 200; i++) {
		snprintf(s, sizeof(s), "%d pls!", i);
		cache_add( i, s, strlen(s) + 1 );
		printf("Put in '%s'\n", cache_find(i));
	}

	for (int i = 0; i < 200; i++) {
		printf("%s\n", cache_find(i));
	}

	return 0;
}
