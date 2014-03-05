#ifndef CACHE_H
#define CACHE_H

#define DEFAULT_CACHE_SIZE ((size_t) 256)

typedef struct cache_header_s Cache;

int gen_id(Cache *cache);
char *cache_find(Cache *cache, int id, size_t *len);
int cache_add(Cache *cache, char *data, size_t len);
int cache_sync(Cache *cache);
Cache *cache_init(int fd, size_t size);

#endif
