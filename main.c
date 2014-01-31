#include <stdio.h>
#include <err.h>
#define ASSERT(a) do { if (!(a)) errx(1, __FILE__ ":%d: " #a, __LINE__); } while (0);



/* 75 students, 5 tutorials, 20 capacity.  Tutorial 0 is very popular. */

int const vote[75][5] = {
#include "data.txt"  /* made with `./mkdata` */
};

int capacity[5] = { 20, 20, 20, 20, 20 };

int head[5];
int prev[75];
int next[75];



void move(int const s, int const ta, int const tb)
{
	ASSERT(ta != tb);
	int sn = next[s];
	int sp = prev[s];

	/* remove s from ta */
	if (sp >= 0)
		next[sp] = sn;
	else
		head[ta] = sn;

	if (sn >= 0)
		prev[sn] = sp;

	/* add s to tb */
	prev[s] = -1;
	next[s] = head[tb];
	prev[head[tb]] = s;
	head[tb] = s;

	capacity[ta]++;
	capacity[tb]--;
}



void show(void)
{
	printf("--------------------\n");
	int cost = 0, mem = 0;
	for (int t = 0; t < 5; t++) {
		int cost1 = 0, mem1 = 0;
		printf("%d:", t);
		for (int s = head[t]; s >=0; s = next[s]) {
			printf(" %d", s);
			cost1 += vote[s][t];
			mem1++;
		}
		printf("; c=%d, m=%d\n", cost1, mem1);
		cost += cost1;
		mem += mem1;
	}
	printf("cost = %d, members = %d\n", cost, mem);
}



int seen[5];
int value[5];



int dfs(int const ta, int const delta) {

	if (seen[ta]) { /* cycle */
		if (delta < value[ta]) { /* useful cycle */
			printf("cycle: %d", ta);
			seen[ta] = 0; /* mark where cycle closes */
			return 1;
		}
		return 0; /* cycle not useful */
	}

	/* this node is new */
	seen[ta] = 1;
	value[ta] = delta;

	if (head[ta] < 0) /* nobody here */
		return 0; /* handle PATH instead of CYCLE */

	for (int tb = 0; tb < 5; tb++) {
		if (ta == tb) /* do not move to same node */
			continue;

		int who = -1, dmin;
		for (int s = head[ta]; s >= 0; s = next[s]) {
			int d = vote[s][tb] - vote[s][ta];
			if (who < 0 || d < dmin) {
				who = s;
				dmin = d;
			}
		}
		ASSERT(who >= 0);

		if (dfs(tb, delta + dmin)) {

			/* found a path or cycle */
			printf(" <-%d-- %d", who, ta);
			move(who, ta, tb);

			if (seen[ta]) {
				seen[ta] = 0;
				return 1; /* leave loop, still moving cycle */
			}

			/* this ta is where cycle into tb started */
			printf("\n");
		}
	}
	return 0;
}



int main(void)
{

	for (int t = 0; t < 5; t++)
		head[t] = -1;

	for (int t = 0, s = 0; s < 75; s++) {
		while (capacity[t] == 0)
			t++;
		capacity[t]--;
		prev[s] = -1;
		next[s] = head[t];
		prev[head[t]] = s;
		head[t] = s;
	}
	show();

	for (int t = 0; t < 5; t++)
		seen[t] = 0;
	for (int t = 0; t < 5; t++) {
		while (dfs(t, 0)) {
			for (int t = 0; t < 5; t++)
				seen[t] = 0;
			printf("\n");
		}
	}
	show();

	return 0;
}
