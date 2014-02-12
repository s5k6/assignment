#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include "parser.h"


fixed const *readVotes(int const tutors, char const * const slot[])
{
	int alloc = 10,
		count = 0;

	char const **name = malloc((size_t)alloc * sizeof(char*));
	assert(name);
	int **vote = malloc((size_t)alloc * sizeof(int*));
	assert(vote);
	int *capacity = malloc((size_t)tutors * sizeof(int));
	assert(capacity);
	for (int t = 0; t < tutors; t++) capacity[t] = 20;

	size_t size = 100;
	char *buf = malloc(size);

	ssize_t len;
	while ((len = getline(&buf, &size, stdin)) > -1) {
		char *saveptr;
		name[count] = strtok_r(buf, " \t\n\0", &saveptr);
		if (name[count]) {
			int seen[tutors], j = 0;
			char *v;
			vote[count] = malloc((size_t)tutors * sizeof(int));
			assert(vote[count]);

			for (int k = 0; k < tutors; k++) {
				seen[k] = 0;
				vote[count][k] = -1;
			}

			while ((v = strtok_r(NULL, " \t\n\0", &saveptr)) != NULL) {
				if (j == tutors) errx(1, "excessive choice `%s`", v);
				for (int k = 0; k < tutors; k++) {
					if (strcmp(slot[k], v) == 0) {
						if (seen[k]) errx(1, "repeated `%s`", v);
						vote[count][j] = k;
						seen[k] = 1;
						break;
					}
				}
				if (vote[count][j] < 0)
					errx(1, "invalid choice `%s`", v);
				j++;
			}
			buf = malloc(size);
			count++;
			if (count == alloc) {
				alloc *= 2;
				name = realloc(name, (size_t)alloc * sizeof(char*));
				assert(name);
				vote = realloc(vote, (size_t)alloc * sizeof(int*));
				assert(vote);
			}
		}
	}

	/* calculate cost from vote */
	int **cost = malloc((size_t)count * sizeof(int*));
	assert(cost);
	for (int s = 0; s < count; s++) {
		cost[s] = malloc((size_t)tutors * sizeof(int));
		assert(cost[s]);
		for (int v = 0; v < tutors; v++) {
			assert(vote[s][v] >= 0);
			cost[s][vote[s][v]] = v;
		}
	}

	fixed tmp = {
		.students = count,
		.tutors = (int)tutors,
		.name = name,
		.slot = slot,
		.vote = vote,
		.cost = cost,
		.capacity = capacity,
	};

	fixed *fi = malloc(sizeof(fixed));
	assert(fi);
	memcpy(fi, &tmp, sizeof(fixed));
	return fi;
}
