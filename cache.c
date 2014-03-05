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

static cache_node *cache_sentinel = NULL;
#define CACHE_TAIL (cache_sentinel->next)
static cache_node *cache_start = NULL;

int gen_id(void)
{
	static int id = 0;
	return id++;
}

char *cache_find(int id)
{
	if (cache_sentinel == NULL || CACHE_TAIL == NULL) {
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
	/* cache not initialized or the data doesn't fit at all */
	if ( cache_sentinel == NULL || sizeof(cache_node) + len >= CACHE_SIZE ) {
		return NULL;
	}

	/* First addition */
	if (CACHE_TAIL == NULL) {
		CACHE_TAIL = cache_start;
		CACHE_TAIL->id = id;
		CACHE_TAIL->len = len;
		CACHE_TAIL->next = CACHE_TAIL;
		memcpy( CACHE_TAIL->data, data, len );
		return CACHE_TAIL->data;
	}

	cache_node *prev = CACHE_TAIL;
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
	CACHE_TAIL = prev->next = curr;
	return (char *) (curr + 1);
}

void cache_init(int fd)
{
	cache_sentinel = mmap( NULL, CACHE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
	if (cache_sentinel == MAP_FAILED) {
		printf( "MAP_FAILED!\n%s\n", strerror(errno) );
		exit(1);
	}
	cache_start = cache_sentinel + 1;
	cache_sentinel->len = 0;
	cache_sentinel->id = -1;
}

