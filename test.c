#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cache.h"

int main(void)
{
	int fd = open("tmpfile",
			O_RDWR | O_CREAT,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	ftruncate(fd, DEFAULT_CACHE_SIZE);
	assert(fd != -1);
	Cache *cache = cache_init(fd, 256);
	assert(cache != NULL);

	char buf[1024];
	int id;
	for (int i = 0; i < 8; i++) {
		snprintf(buf, 1024, "#%d Fill %d#", i, i);
		id = cache_add(cache, buf, strlen(buf) + 1);
		assert(id != -1);
		id -= i;
	}

	for (int i = id; i < id + 8; i++) {
		printf("%d: %s\n", i, cache_find(cache, i, NULL));
	}

	cache_sync(cache);

	return 0;
}
