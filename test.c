#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "cache.h"

int main(void)
{
	int fd = open("tmpfile",
			O_RDWR | O_CREAT | O_TRUNC,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	ftruncate(fd, CACHE_SIZE);
	assert(fd != -1);
	cache_init(fd);

	int N =32;

	char s[100];
	for (int i = 0; i < N; i++) {
		snprintf(s, sizeof(s), "%dpls", N-i);
		cache_add( i, s, strlen(s) + 1 );
	}

	for (int i = 0; i < N; i++) {
		printf("%s\n", cache_find(N-i-1));
	}

	return 0;
}
