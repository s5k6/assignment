#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <err.h>
#include "parser.h"


int main(int argc, char const * const argv[])
{
	char const *const *slot = argv + 1;

	fixed const *fi = readVotes(argc - 1, slot);

	warnx("count = %d", fi->students);
	for (int i = 0; i < fi->students; i++) {
		printf("%3s ", fi->name[i]);
		for (int j = 0; j < fi->tutors; j++)
			printf(" %s", fi->vote[i][j] >= 0 ? fi->slot[fi->vote[i][j]] : "~");
		printf("\n");
	}
}
