#ifndef CACHE_H
#define CACHE_H

#define CACHE_SIZE ((size_t) 512)

char *cache_find(int id);
char *cache_add(int id, char *data, size_t len);
void cache_init(int fd);

#endif
