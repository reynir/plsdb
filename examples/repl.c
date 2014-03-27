#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "../cache.h"

void repl(Cache *cache)
{
	while (1) {
		char cmd;
		scanf("%c", &cmd);
		if (cmd == 'q') {
			break;
		} else if (cmd == 'f') {
			int id = -1;
			scanf("%d", &id);
			char *res = cache_find(cache, id, NULL);
			if (res == NULL) {
				printf("id %d was not found\n", id);
			} else {
				printf("Found %d: '%s'\n", id, res);
			}
		} else if (cmd == 'a') {
			char *s = readline("> ");
			int id;
			if (s == NULL) {
				puts("\n");
				break;
			}
			if ((id = cache_add(cache, s, strlen(s) + 1)) != -1) {
				printf("Added to cache as %d\n", id);
			} else {
				printf("Failed to add to cache!\n");
			}
			free(s);
			cache_sync(cache);
		}
	}
}

int main(int argc, char *argv[])
{
	Cache *cache = cache_init_file("tmpfile", DEFAULT_CACHE_SIZE);
	assert(cache != NULL);

	if (argc > 1) {
		printf("TODO: arguments 'n stuff\n");
		exit(0);
	} else {
		repl(cache);
	}

	return 0;
}
