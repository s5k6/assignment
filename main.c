#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <err.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>



int students, tutors, **vote, **cost;
char **name, **slot;



#define forTutor(t) for (int t = 0; t < tutors; t++)
#define forStudent(s) for (int s = 0; s < students; s++)
#define forMember(s,t) for (int s = head[t]; s >= 0; s = next[s])


void readVotes(void)
{
	int alloc = 10;
	students = 0;

	name = malloc((size_t)alloc * sizeof(char*));
	assert(name);
	vote = malloc((size_t)alloc * sizeof(int*));
	assert(vote);

	size_t size = 100;
	char *buf = malloc(size);

	ssize_t len;
	while ((len = getline(&buf, &size, stdin)) > -1) {
		char *saveptr;
		name[students] = strtok_r(buf, " \t\n\0", &saveptr);
		if (name[students]) {
			int seen[tutors], j = 0;
			char *v;
			vote[students] = malloc((size_t)tutors * sizeof(int));
			assert(vote[students]);

			forTutor(k) {
				seen[k] = 0;
				vote[students][k] = -1;
			}

			while ((v = strtok_r(NULL, " \t\n\0", &saveptr)) != NULL) {
				if (j == tutors) errx(1, "excessive choice `%s`", v);
				forTutor(k) {
					if (strcmp(slot[k], v) == 0) {
						if (seen[k]) errx(1, "repeated `%s`", v);
						vote[students][j] = k;
						seen[k] = 1;
						break;
					}
				}
				if (vote[students][j] < 0)
					errx(1, "invalid choice `%s`", v);
				j++;
			}
			buf = malloc(size);
			students++;
			if (students == alloc) {
				alloc *= 2;
				name = realloc(name, (size_t)alloc * sizeof(char*));
				assert(name);
				vote = realloc(vote, (size_t)alloc * sizeof(int*));
				assert(vote);
			}
		}
	}

	/* calculate cost from vote */
	cost = malloc((size_t)students * sizeof(int*));
	assert(cost);
	forStudent(s) {
		cost[s] = malloc((size_t)tutors * sizeof(int));
		assert(cost[s]);
		forTutor(v) {
			assert(vote[s][v] >= 0);
			cost[s][vote[s][v]] = v;
		}
	}
}



int *maximum, *capacity, *head, *prev, *next;



void integrity(void)
{
	int tot = 0;
	int checked[students];

	forStudent(i) {
		checked[i] = 0;
	}

	forTutor(t) {
		if (head[t] >= 0) {
			assert(prev[head[t]] == -1);
			forMember(s,t) {
				tot++;
				assert(checked[s] == 0);
				checked[s] = 1;
				assert((s == head[t] && prev[s] == -1) || next[prev[s]] == s);
				assert(next[s] == -1 || prev[next[s]] == s);
			}
		} else assert(head[t] == -1);
	}
	assert(tot = students);
}



void initialAssignment(void)
{
	head = malloc((size_t)tutors * sizeof(int)); assert(head);
	capacity = malloc((size_t)tutors * sizeof(int)); assert(capacity);
	prev = malloc((size_t)students * sizeof(int)); assert(prev);
	next = malloc((size_t)students * sizeof(int)); assert(next);

	forTutor(t) {
		head[t] = -1;
		capacity[t] = maximum[t];
	}

	forStudent(s) {
		int c = tutors, a = -1;
		forTutor(t) {
            if (vote[s][t] < c && capacity[t] > 0) {
                a = t;
                c = vote[s][t];
            }
        }
        assert(a > -1);

        capacity[a]--;
        prev[s] = -1;
        next[s] = head[a];
        if (head[a] >= 0)
            prev[head[a]] = s;
        head[a] = s;
    }
	integrity();
}



void show(void)
{
	int sum = 0, mem = 0;
	forTutor(t) {
		int sum1 = 0, mem1 = 0;
		printf("%s:", slot[t]);
		forMember(s,t) {
			printf(" %s", name[s]);
			sum1 += cost[s][t];
			mem1++;
		}
		printf("; sum=%d, memb=%d\n", sum1, mem1);
		sum += sum1;
		mem += mem1;
		assert(mem1 + capacity[t] == maximum[t]);
	}
	printf("total sum = %d, members = %d\n", sum, mem);
	printf("--------------------\n");
}



/* This actually performs the relocation of a student, and updates all the
   data structures.  */

void move(int const s, int const ta, int const tb)
{
	assert(ta != tb);
	assert(0 <= ta && ta < tutors && 0 <= tb && tb < tutors);
	assert(0 <= s);
	assert(s < students);
	assert(head[ta] >= 0);

	int j = s;
	while (prev[j] >= 0) j = prev[j];
	assert(head[ta] = j);

	int const sn = next[s];
	int const sp = prev[s];
	next[s] = head[tb];
	prev[s] = -1;

	if (sp < 0) {
		assert(head[ta] == s);
		head[ta] = sn;
	} else {
		assert(head[ta] != s);
		next[sp] = sn;
	}

	if (sn >= 0) prev[sn] = sp;

	if (head[tb] >= 0) prev[head[tb]] = s;
	head[tb] = s;


	capacity[ta]++;
	capacity[tb]--;

	integrity();
}






int *seen;
int *delta;
int cycle;


int push(int ta, int d) {

	if (seen[ta]) {
		if (d < delta[ta]) {
			printf("cycle%d: %d", d - delta[ta], ta);
			cycle = ta;
			return 1;
		}
		return 0;
	}

	if (capacity[ta] > 0) {
		//printf("capacity[%d] = %d\n", ta, capacity[ta]);
		if (d < 0) {
			printf("path%d: %d", d, ta);
			cycle = -1;
			return 1;
		}
		return 0;
	}

	/* too many students here */

	seen[ta] = 1;
	delta[ta] = d;

	forTutor(tb) {
		if (ta == tb) continue;

		int w = -1, wc, sc;
		forMember(s,ta) {
			sc = vote[s][tb] - vote[s][ta];
			if (w < 0 || sc < wc) {
				w = s;
				wc = sc;
			}
		}
		if (w < 0) continue;


		if (push(tb, d + wc)) {
			if (cycle < 0) { /* we are on a path */
				if (d < 0) { /* we are on the useful part */
					printf(" <%d- %d", w, ta);
					move(w, ta, tb);
					seen[ta] = 0;
					return 1;
				}
				/* path up to here is useless */
			} else { /* we are on a cycle */
				printf(" <%d- %d", w, ta);
				move(w, ta, tb);
				if (ta != cycle) { /* this is not the start */
					seen[ta] = 0;
					return 1;
				}
				printf("\n");
				/* just closed a cycle */
				break;
			}
		}

	}

	seen[ta] = 0;
	return 0;
}



int main(int argc, char **argv)
{

	tutors = argc - 1;
	slot = argv + 1;

	/* get this from command line as well */
	maximum = malloc((size_t)tutors * sizeof(int)); assert(maximum);
	forTutor(t) maximum[t] = 20;

	readVotes();

	initialAssignment();
	show();


	seen = malloc((size_t)tutors * sizeof(int)); assert(seen);
	delta = malloc((size_t)tutors * sizeof(int)); assert(delta);

	forTutor(t)
		seen[t] = 0;

	forTutor(ta) {
		seen[ta] = 1;
		delta[ta] = 0;
		forTutor(tb) {
			if (ta == tb) continue;

			int w = -1, wc, sc;
			forMember(s,ta) {
				sc = vote[s][tb] - vote[s][ta];
				if (w < 0 || sc < wc) {
					w = s;
					wc = sc;
				}
			}
			if (w < 0) continue;


			if (wc < 0) {
				if (push(tb, wc)) {
					move(w, ta, tb);
					printf(" <%d- %d top\n", w, ta);
					break;
				}
			}
		}
		seen[ta] = 0;
	}


	show();

	return 0;
}
