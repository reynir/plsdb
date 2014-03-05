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
	size_t next;
	size_t len;
	char data[];
} cache_node;

static cache_node *cache_sentinel = NULL;
#define CACHE_TAIL (cache_sentinel->next)
static cache_node *cache_start = NULL;

#define to_rel(x) ((char *) (x) - (char *) cache_sentinel)
#define from_rel(x) ((cache_node *) ((char *) cache_sentinel + (x)))

int gen_id(void)
{
	static int id = 0;
	return id++;
}

char *cache_find(int id)
{
	if (cache_sentinel == NULL || CACHE_TAIL == 0) {
		return NULL;
	}
	cache_node *curr = cache_start;
	if (curr->id == id) {
		return curr->data;
	}

	curr = from_rel(curr->next);
	while (curr != cache_start) {
		if (curr->id == id) {
			return curr->data;
		}
		curr = from_rel(curr->next);
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
	if (CACHE_TAIL == 0) {
		cache_node *tail = cache_start;
		CACHE_TAIL = to_rel(tail);
		tail->id = id;
		tail->len = len;
		tail->next = to_rel(tail);
		memcpy( tail->data, data, len );
		return tail->data;
	}

	cache_node *prev = from_rel(CACHE_TAIL);
	cache_node *next = from_rel(prev->next);
	cache_node *curr = (cache_node *) (prev->data + prev->len);

	/* if data doesn't fit after curr */
	if ( curr->data + len >= (char *) cache_start + CACHE_SIZE ) {
		curr = cache_start;
		next = from_rel(curr->next);
	}


	/* Free nodes if we need more space */
	while ( curr <= next &&
			/* Not enough space */
			curr->data + len >= (char *) next ) {
		/* Free the next cache_node */
		next = from_rel(prev->next = next->next);
	}

	memcpy( curr->data, data, len );
	curr->next = to_rel(next);
	curr->id = id;
	curr->len = len;
	CACHE_TAIL = prev->next = to_rel(curr);
	return curr->data;
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

