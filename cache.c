#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "cache.h"

typedef struct cache_node {
	int id;
	struct cache_node *next;
	size_t len;
	char data[];
} cache_node;

static cache_node *cache_start = NULL;
static cache_node *cache_last = NULL;

int gen_id(void)
{
	static int id = 0;
	return id++;
}

char *cache_find(int id)
{
	if (cache_start == NULL) {
		return NULL;
	}
	cache_node *curr = cache_start;
	if (curr->id == id) {
		return curr->data;
	}

	curr = curr->next;
	while (curr != cache_start) {
		if (curr->id == id) {
			return curr->data;
		}
		curr = curr->next;
	}
	return NULL;
}

char *cache_add(int id, char *data, size_t len)
{
	assert( strlen(data) + 1 == len );
	/* return NULL if the data doesn't fit at all */
	if ( sizeof(cache_node) + len >= CACHE_SIZE ) {
		return NULL;
	}

	/* First addition */
	if (cache_last == NULL) {
		cache_last = cache_start;
		cache_last->id = id;
		cache_last->len = len;
		cache_last->next = cache_last;
		memcpy( cache_last->data, data, len );
		return (char *) (cache_last + 1);
	}

	cache_node *prev = cache_last;
	cache_node *next = prev->next;
	cache_node *curr = (cache_node *) (prev->data + prev->len) + 1;

	/* if data doesn't fit after curr */
	if ( curr->data + len >= (char *) cache_start + CACHE_SIZE ) {
		curr = cache_start;
		next = curr->next;
	}


	/* Update the next pointers */
	while ( curr <= next &&
			/* Not enough space */
			curr->data + len >= (char *) next ) {
		/* Free the next cache_node */
		next = prev->next = next->next;
	}

	memcpy( curr->data, data, len );
	curr->next = next;
	curr->id = id;
	curr->len = len;
	cache_last = prev->next = curr;
	return (char *) (curr + 1);
}

void cache_init(int fd)
{
	cache_start = mmap( NULL, CACHE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
	if (cache_start == MAP_FAILED) {
		printf( "MAP_FAILED!\n%s\n", strerror(errno) );
		exit(1);
	}
	cache_last = NULL;
}

