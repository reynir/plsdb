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

#define CACHE_MAGIC 0x0cded26c
#define CACHE_VERSION 2

typedef struct cache_node_s {
	int id;
	size_t next;
	size_t len;
	char data[];
} cache_node;

typedef struct cache_header_s {
	int magic;
	int version;
	int counter;
	size_t size;
	size_t tail;
	cache_node start[];
} cache_header;

// Not sure which one is better...
//#define to_rel(header, x) ((char *) (x) - (char *) (header))
size_t to_rel(cache_header *header, cache_node *x)
{
	return (char *) x - (char *) header;
}
//#define from_rel(header, x) ((cache_node *) ((char *) (header) + (x)))
cache_node *from_rel(cache_header *header, size_t x)
{
	return (cache_node *) ((char *) header + x);
}

int gen_id(cache_header *header)
{
	int res = header->counter++;
	if (header->counter < 0)
		header->counter = 0;
	return res;
}

char *cache_find(cache_header *header, int id, size_t *len)
{
	if (header == NULL || header->tail == 0) {
		return NULL;
	}
	cache_node *curr = header->start;
	if (curr->id == id) {
		if (len != NULL)
			*len = curr->len;
		return curr->data;
	}

	curr = from_rel(header, curr->next);
	while (curr != header->start) {
		if (curr->id == id) {
			if (len != NULL)
				*len = curr->len;
			return curr->data;
		}
		curr = from_rel(header, curr->next);
	}
	return NULL;
}

int cache_add(cache_header *header, char *data, size_t len)
{
	assert( strlen(data) + 1 == len );
	/* cache not initialized or the data doesn't fit at all */
	if ( header == NULL || sizeof(cache_node) + len >= header->size - sizeof(*header) ) {
		return -1;
	}
	int id = gen_id(header);

	/* First addition */
	if (header->tail == 0) {
		cache_node *tail = header->start;
		header->tail = to_rel(header, tail);
		tail->id = id;
		tail->len = len;
		tail->next = to_rel(header, tail);
		memcpy( tail->data, data, len );
		return id;
	}

	cache_node *prev = from_rel(header, header->tail);
	cache_node *next = from_rel(header, prev->next);
	cache_node *curr = (cache_node *) (prev->data + prev->len);

	/* if data doesn't fit after curr */
	if ( curr->data + len >= (char *) header + header->size ) {
		curr = header->start;
		next = from_rel(header, curr->next);
	}


	/* Free nodes if we need more space */
	while ( curr <= next &&
			/* Not enough space */
			curr->data + len >= (char *) next ) {
		/* Free the next cache_node */
		next = from_rel(header, prev->next = next->next);
	}

	memcpy( curr->data, data, len );
	curr->next = to_rel(header, next);
	curr->id = id;
	curr->len = len;
	header->tail = prev->next = to_rel(header, curr);
	return id;
}

int cache_sync(cache_header *header)
{
	return msync(header, header->size, MS_SYNC);
}

cache_header *cache_init(int fd, size_t size)
{
	cache_header *header;
	if (fd == -1) {
		header = mmap( NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 );
	} else {
		header = mmap( NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
	}

	if (header == MAP_FAILED) {
		printf( "MAP_FAILED!\n%s\n", strerror(errno) );
		exit(1);
	}
	/* TODO: If header->magic !=CACHE_MAGIC then check if the file is
	 * "empty" (all zeros) */
	if (header->magic != 0 && header->version != 0 &&
			(header->magic != CACHE_MAGIC || header->version != CACHE_VERSION)) {
		fprintf(stderr, header->magic != CACHE_MAGIC
				? "Not a cache!!!\n"
				: "Outdated cache!!!\n");
		munmap(header, size);
		return NULL;
	}
	header->magic = CACHE_MAGIC;
	header->version = CACHE_VERSION;
	header->size = size;

	return header;
}

cache_header *cache_init_file(char *filename, size_t size)
{
	int fd = open(filename,
			O_RDWR | O_CREAT,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	assert(fd != -1);
	ftruncate(fd, size);
	return cache_init(fd, size);
}

cache_header *cache_init_ram(size_t size)
{
	return cache_init(-1, size);
}
