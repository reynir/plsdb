#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "cache.h"

void repl()
{
	while (1) {
		char cmd;
		scanf("%c", &cmd);
		if (cmd == 'q') {
			break;
		} else if (cmd == 'f') {
			int id = -1;
			scanf("%d", &id);
			char *res = cache_find(id, NULL);
			if (res == NULL) {
				printf("id %d was not found\n", id);
			} else {
				printf("Found %d: '%s'\n", id, res);
			}
		} else if (cmd == 'a') {
			int id = gen_id();
			char *s = readline("> ");
			if (s == NULL) {
				puts("\n");
				break;
			}
			if (cache_add(id, s, strlen(s) + 1) != NULL) {
				printf("Added to cache as %d\n", id);
			} else {
				printf("Failed to add to cache!\n");
			}
			free(s);
			cache_sync();
		}
	}
}

int main(int argc, char *argv[])
{
	int fd = open("tmpfile",
			O_RDWR | O_CREAT,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	ftruncate(fd, CACHE_SIZE);
	assert(fd != -1);
	cache_init(fd);

	if (argc > 1) {
		printf("TODO: arguments 'n stuff\n");
		exit(0);
	} else {
		repl();
	}

	return 0;
}
