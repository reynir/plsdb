#ifndef CACHE_H
#define CACHE_H

#include <stddef.h>

#define DEFAULT_CACHE_SIZE ((size_t) 256)

typedef struct cache_header_s Cache;

int gen_id(Cache *cache);
char *cache_find(Cache *cache, int id, size_t *len);
int cache_add(Cache *cache, char *data, size_t len);
int cache_sync(Cache *cache);
Cache *cache_init_file(char *filename, size_t size);
Cache *cache_init_ram(size_t size);

#endif
