#include <stdio.h>
#include <err.h>
#define ASSERT(a) if (!(a)) errx(1, __FILE__ ":%d: ASSERTION " #a, __LINE__);



#include "data.inc"  /* made with `./mkdata` */

int capacity[TUTORS];

int head[TUTORS];
int prev[STUDENTS];
int next[STUDENTS];



void integrity(void)
{
	int tot = 0;
	int seen[STUDENTS];

	for (int i = 0; i < STUDENTS; i++) {
		seen[i] = 0;
	}

	for (int t = 0; t < TUTORS; t++) {
		if (head[t] >= 0) {
			ASSERT(prev[head[t]] == -1);
			for (int s = head[t]; s >= 0; s = next[s]) {
				tot++;
				ASSERT(seen[s] == 0);
				seen[s] = 1;
				ASSERT((s == head[t] && prev[s] == -1) || next[prev[s]] == s);
				ASSERT(next[s] == -1 || prev[next[s]] == s);
			}
		} else ASSERT(head[t] == -1);
	}
	ASSERT(tot = STUDENTS);
}



void show(void)
{
	int cost = 0, mem = 0;
	for (int t = 0; t < TUTORS; t++) {
		int cost1 = 0, mem1 = 0;
		printf("@%d:", t);
		for (int s = head[t]; s >=0; s = next[s]) {
			printf(" %d", s);
			cost1 += vote[s][t];
			mem1++;
		}
		printf("; cost=%d, memb=%d\n", cost1, mem1);
		cost += cost1;
		mem += mem1;
		ASSERT(mem1 + capacity[t] == CAPACITY);
	}
	printf("total cost = %d, members = %d\n", cost, mem);
	printf("--------------------\n");
}



/* This actually performs the relocation of a student, and updates all the
   data structures.  */

void move(int const s, int const ta, int const tb)
{
	ASSERT(ta != tb);
	ASSERT(0 <= ta && ta < TUTORS && 0 <= tb && tb < TUTORS);
	ASSERT(0 <= s);
	ASSERT(s < STUDENTS);
	ASSERT(head[ta] >= 0);

	int j = s;
	while (prev[j] >= 0) j = prev[j];
	ASSERT(head[ta] = j);

	int const sn = next[s];
	int const sp = prev[s];
	next[s] = head[tb];
	prev[s] = -1;

	if (sp < 0) {
		ASSERT(head[ta] == s);
		head[ta] = sn;
	} else {
		ASSERT(head[ta] != s);
		next[sp] = sn;
	}

	if (sn >= 0) prev[sn] = sp;

	if (head[tb] >= 0) prev[head[tb]] = s;
	head[tb] = s;


	capacity[ta]++;
	capacity[tb]--;

	integrity();
}



void initialAssignment(void)
{
	for (int t = 0; t < TUTORS; t++) {
		head[t] = -1;
		capacity[t] = CAPACITY;
	}

	for (int s = 0; s < STUDENTS; s++) {
		int c = TUTORS, a = -1;
		for (int t = 0; t < TUTORS; t++) {
            if (vote[s][t] < c && capacity[t] > 0) {
                a = t;
                c = vote[s][t];
            }
        }
        ASSERT(a > -1);

        capacity[a]--;
        prev[s] = -1;
        next[s] = head[a];
        if (head[a] >= 0)
            prev[head[a]] = s;
        head[a] = s;
    }
	integrity();
}



int seen[TUTORS];
int cycle;


int push(int ta, int delta) {

	if (seen[ta]) {
		if (delta < 0) {
			printf("cycle%d: %d", delta, ta);
			cycle = ta;
			return 1;
		}
		return 0;
	}

	if (capacity[ta] > 0) {
		//printf("capacity[%d] = %d\n", ta, capacity[ta]);
		if (delta < 0) {
			printf("path%d: %d", delta, ta);
			cycle = -1;
			return 1;
		}
		return 0;
	}

	/* too many students here */

	seen[ta] = 1;

	int n = -1;
	for (int s = head[ta]; s >= 0; s = n) {
		n = next[s];
		for (int tb = 0; tb < TUTORS; tb++) {
			if (ta == tb) continue;

			if (push(tb, delta + vote[s][tb] - vote[s][ta])) {
				if (cycle < 0) { /* we are on a path */
					if (delta < 0) { /* we are on the useful part */
						printf(" <%d- %d", s, tb);
						move(s, ta, tb);
						seen[ta] = 0;
						return 1;
					}
					/* path up to here is useless */
				} else { /* we are on a cycle */
					printf(" <%d- %d", s, tb);
					move(s,ta, tb);
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
	}

	seen[ta] = 0;
	return 0;
}



int main(void)
{
	initialAssignment();
	show();

	for (int t = 0; t < TUTORS; t++)
		seen[t] = 0;

	for (int ta = 0; ta < TUTORS; ta++) {
		int n = -1;
		seen[ta] = 1;
		for (int s = head[ta]; s >= 0; s = n) {
			n = next[s];
			printf("init: %d\n", s);
			for (int tb = 0; tb < TUTORS; tb++) {
				if (ta == tb) continue;
				int d = vote[s][tb] - vote[s][ta];
				if (d < 0) {
					if (push(tb, d)) {
						move(s, ta, tb);
						printf(" <%d- %d top\n", s, tb);
						break;
					}
				}
			}
		}
		seen[ta] = 0;
	}


	show();

	return 0;
}
