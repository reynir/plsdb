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
	ftruncate(fd, CACHE_SIZE);
	assert(fd != -1);
	cache_init(fd);

	char buf[1024];
	for (int i = 0; i < 32; i++) {
		snprintf(buf, 1024, "#%d Fill %d#", i, i);
		assert(cache_add(i, buf, strlen(buf) + 1) != NULL);
	}

	for (int i = 0; i < 32; i++) {
		printf("%s\n", cache_find(i, NULL));
	}

	cache_sync();

	return 0;
}
