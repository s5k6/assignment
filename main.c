#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <assert.h>
#include "parser.h"


#define forTutors(t) for (int t = 0; t < (int)fi->tutors; t++)
#define forStudents(s) for (int s = 0; s < (int)fi->students; s++)
#define forMembers(s,t) for (int s = st->head[t]; s >= 0; s = st->next[s])

typedef struct {
	int *capacity;
	int *head;
	int *prev;
	int *next;
	int *seen;
	int *delta;
	int cycle;
} status;



void integrity(fixed const *fi, status const *st)
{
	int tot = 0;
	int checked[fi->students];

	forStudents(i) checked[i] = 0;

	forTutors(t) {
		if (st->head[t] >= 0) {
			assert(st->prev[st->head[t]] == -1);
			forMembers(s,t) {
				tot++;
				assert(checked[s] == 0);
				checked[s] = 1;
				assert((s == st->head[t] && st->prev[s] == -1) || st->next[st->prev[s]] == s);
				assert(st->next[s] == -1 || st->prev[st->next[s]] == s);
			}
		} else assert(st->head[t] == -1);
	}
	assert(tot = fi->students);
}



void show(fixed const *fi, status *st)
{
	int sum = 0, mem = 0;
	forTutors(t) {
		int sum1 = 0, mem1 = 0;
		printf("%s:", fi->slot[t]);
		forMembers(s,t) {
			printf(" %s", fi->name[s]);
			sum1 += fi->cost[s][t];
			mem1++;
		}
		printf("; sum=%d, memb=%d\n", sum1, mem1);
		sum += sum1;
		mem += mem1;
		assert(mem1 + st->capacity[t] == fi->capacity[t]);
	}
	printf("total sum = %d, members = %d\n", sum, mem);
	printf("--------------------\n");
}



status *mkstatus(fixed const *fi)
{
	status *st = malloc(sizeof(status));
	assert(st);

	st->capacity = malloc((size_t)fi->tutors * sizeof(int));
	assert(st->capacity);
	st->head = malloc((size_t)fi->tutors * sizeof(int));
	assert(st->head);
	st->prev = malloc((size_t)fi->students * sizeof(int));
	assert(st->prev);
	st->next = malloc((size_t)fi->students * sizeof(int));
	assert(st->next);

	forTutors(t) {
		st->head[t] = -1;
		st->capacity[t] = fi->capacity[t];
	}
	return st;
}



status *initialAssignment1(fixed const *fi)
{
	status *st = mkstatus(fi);

	int a = 0;
	forStudents(s) {
		while (st->capacity[a] < 1) a++;
        st->capacity[a]--;
        st->prev[s] = -1;
        st->next[s] = st->head[a];
        if (st->head[a] >= 0)
            st->prev[st->head[a]] = s;
        st->head[a] = s;
    }
	integrity(fi, st);

	return st;
}



status *initialAssignment2(fixed const *fi)
{
	status *st = mkstatus(fi);

	forStudents(s) {
		int c = -1, a = -1;
		forTutors(t) {
			if ((c < 0 || fi->cost[s][t] < c) && st->capacity[t] > 0) {
                a = t;
                c = fi->cost[s][t];
            }
        }
        assert(a > -1);

        st->capacity[a]--;
        st->prev[s] = -1;
        st->next[s] = st->head[a];
        if (st->head[a] >= 0)
            st->prev[st->head[a]] = s;
        st->head[a] = s;
    }
	integrity(fi, st);

	return st;
}



/* This actually performs the relocation of a student, and updates all the
   data structures.  */

void move(fixed const *const fi, status *const st, int const s, int const ta, int const tb)
{
	assert(ta != tb);
	assert(0 <= ta && ta < fi->tutors && 0 <= tb && tb < fi->tutors);
	assert(0 <= s);
	assert(s < fi->students);
	assert(st->head[ta] >= 0);

	int j = s;
	while (st->prev[j] >= 0) j = st->prev[j];
	assert(st->head[ta] == j);

	int const sn = st->next[s];
	int const sp = st->prev[s];
	st->next[s] = st->head[tb];
	st->prev[s] = -1;

	if (sp < 0) {
		assert(st->head[ta] == s);
		st->head[ta] = sn;
	} else {
		assert(st->head[ta] != s);
		st->next[sp] = sn;
	}

	if (sn >= 0) st->prev[sn] = sp;

	if (st->head[tb] >= 0) st->prev[st->head[tb]] = s;
	st->head[tb] = s;

	st->capacity[ta]++;
	st->capacity[tb]--;

	integrity(fi, st);
}



int push(fixed const *fi, status *st, int ta, int d)
{
	if (st->seen[ta]) {
		if (d < st->delta[ta]) {
			printf("cycle%d: %d", d - st->delta[ta], ta);
			st->cycle = ta;
			return 1;
		}
		return 0;
	}

	if (st->capacity[ta] > 0) {
		//printf("st->capacity[%d] = %d\n", ta, st->capacity[ta]);
		if (d < 0) {
			printf("path%d: %d", d, ta);
			st->cycle = -1;
			return 1;
		}
		return 0;
	}

	// too many students here

	st->seen[ta] = 1;
	st->delta[ta] = d;

	forTutors(tb) {
		if (ta == tb) continue;

		int w = -1, wc, sc;
		forMembers(s,ta) {
			sc = fi->cost[s][tb] - fi->cost[s][ta];
			if (w < 0 || sc < wc) {
				w = s;
				wc = sc;
			}
		}
		if (w < 0) continue;


		if (push(fi, st, tb, d + wc)) {
			if (st->cycle < 0) { // we are on a path
				if (d < 0) { // we are on the useful part
					printf(" <=(%d,%d)= %d", w, wc, ta);
					move(fi, st, w, ta, tb);
					st->seen[ta] = 0;
					return 1;
				}
				// path up to here is useless
			} else { // we are on a cycle
				printf(" <=(%d,%d)= %d", w, wc, ta);
				move(fi, st, w, ta, tb);
                if (ta != st->cycle) { // this is not the start
					st->seen[ta] = 0;
					return 1;
				}
				printf("\n");
                // just closed a cycle
				break;
			}
		}

	}

	st->seen[ta] = 0;
	return 0;
}



int assign(fixed const *fi, status *st)
{
	int found = 0;

	forTutors(t) st->seen[t] = 0;

	forTutors(ta) {
		st->seen[ta] = 1;
		st->delta[ta] = 0;
		forTutors(tb) {
			if (ta == tb) continue;

			int w = -1, wc, sc;
			forMembers(s,ta) {
				sc = fi->cost[s][tb] - fi->cost[s][ta];
				if (w < 0 || sc < wc) {
					w = s;
					wc = sc;
				}
			}
			if (w < 0) continue;


			if (wc < 0) {
				if (push(fi, st, tb, wc)) {
					printf(" <=(%d,%d)= %d top\n", w, wc, ta);
					move(fi, st, w, ta, tb);
					found = 1;
					break;
				}
			}
		}
		st->seen[ta] = 0;
	}

	return found;
}



int main(int argc, char const * const argv[])
{
	if (argc < 3) errx(1, "insufficient arguments");

	fixed const *fi = readVotes(argc - 1, argv + 1);
	assert(fi);

	warnx("students = %d", fi->students);

	status *st = initialAssignment1(fi);
	show(fi, st);
	free(st);

	st = initialAssignment2(fi);
	show(fi, st);

	while (assign(fi, st)) show(fi, st);


	show(fi, st);

	return 0;
}
