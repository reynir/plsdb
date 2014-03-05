#ifndef CACHE_H
#define CACHE_H

#define CACHE_SIZE ((size_t) 256)

int gen_id(void);
char *cache_find(int id, size_t *len);
char *cache_add(int id, char *data, size_t len);
int cache_sync(void);
void cache_init(int fd);

#endif
