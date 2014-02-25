#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <err.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdarg.h>


#define ansicol(x) ("\x1b[" x "m")


int students, tutors, places, **vote = NULL, **cost = NULL, **offset = NULL;
char **name = NULL, **slot = NULL;
char const *fnBlue = ansicol("0;34"),
	*fnRed = ansicol("1;31"),
	*fnNorm = ansicol("0");



#define forTutor(t) for (int t = 0; t < tutors; t++)
#define forStudent(s) for (int s = 0; s < students; s++)
#define forMember(s,t) for (int s = head[t]; s >= 0; s = next[s])



/* a variadic function to print messages */

void vout(FILE * stream, const char * fmt, va_list ap) {
  vfprintf(stream, fmt, ap);
}

void info(const char * fmt, ...) {
  va_list ap;
  va_start(ap, fmt); vout(stderr, fmt, ap); va_end(ap);
}



void readVotes(void)
{
	int alloc = 10, lino = 0;
	students = 0;

	name = malloc((size_t)alloc * sizeof(char*));
	assert(name);
	vote = malloc((size_t)alloc * sizeof(int*));
	assert(vote);

	size_t size = 100;
	char *buf = malloc(size);

	ssize_t len;
	while ((len = getline(&buf, &size, stdin)) > -1) {
		lino += 1;
		char *saveptr = strchr(buf, '#');
		if (saveptr) *saveptr = '\0';
		name[students] = strtok_r(buf, " \t\n\0", &saveptr);
		if (name[students]) {
			if (students >= places)
				errx(1, "line %d: Too many students for %d places",
				     lino, places);

			int seen[tutors];

			vote[students] = malloc((size_t)tutors * sizeof(int));
			assert(vote[students]);
			forTutor(k) {
				vote[students][k] = -1;
				seen[k] = 0;
			}

			int j = 0;
			char *tok;
			while ((tok = strtok_r(NULL, " \t\n\0", &saveptr)) != NULL
			       && j < tutors) {
				int k = -1;
				forTutor(t) if (strcmp(slot[t], tok) == 0) {
					k = t;
					break;
				}
				if (k < 0 || seen[k])
					info("line %d: %s: Ignored invalid choice `%s`\n",
					      lino, name[students], tok);
				else {
					vote[students][j++] = k;
					seen[k] = 1;
				}
			}
			if (tok) info("line %d: %s: Ignored excessive choice `%s`\n",
			             lino, name[students], tok);
			if (j < tutors) info("line %d: %s: Extending %d votes\n",
			                      lino, name[students], tutors - j);
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
}



void calcOffsetCost(void) {
	offset = malloc((size_t)students * sizeof(int*));
	assert(offset);
	cost = malloc((size_t)students * sizeof(int*));
	assert(cost);

	forStudent(s) {
		offset[s] = malloc((size_t)tutors * sizeof(int));
		assert(offset[s]);
		cost[s] = malloc((size_t)tutors * sizeof(int));
		assert(cost[s]);

		forTutor(t) offset[s][t] = -1;
		int set = 0;
		forTutor(v) if (vote[s][v] > -1) {
			offset[s][vote[s][v]] = v;
			set++;
		}
		if (set < tutors)
			forTutor(t)
				if (offset[s][t] < 0)
					offset[s][t] = set;

		forTutor(t) {
			int o = offset[s][t];
			//cost[s][t] = o;
			//cost[s][t] = o * o;
			cost[s][t] = o * o * (tutors * students - s);
		}
	}
}



int *maximum = NULL, *capacity = NULL,
	*tutorial = NULL, *head = NULL, *prev = NULL, *next = NULL;



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
				assert(tutorial[s] == t);
				tot++;
				assert(checked[s] == 0);
				checked[s] = 1;
				assert((s == head[t] && prev[s] == -1) || next[prev[s]] == s);
				assert(next[s] == -1 || prev[next[s]] == s);
			}
		} else assert(head[t] == -1);
	}
	assert(tot == students);
}



void initialAssignment(void)
{
	head = malloc((size_t)tutors * sizeof(int));
	assert(head);
	capacity = malloc((size_t)tutors * sizeof(int));
	assert(capacity);
	prev = malloc((size_t)students * sizeof(int));
	assert(prev);
	next = malloc((size_t)students * sizeof(int));
	assert(next);
	tutorial = malloc((size_t)students * sizeof(int));
	assert(tutorial);

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
		assert(a >= 0);

		tutorial[s] = a;
		capacity[a]--;
		prev[s] = -1;
		next[s] = head[a];
		if (head[a] >= 0)
			prev[head[a]] = s;
		head[a] = s;
	}
	integrity();
}


/*
int show(void)
{
	int cst = 0, mem = 0;
	forTutor(t) {
		int cst1 = 0, mem1 = 0;
		info("%s:", slot[t]);
		forMember(s,t) {
			assert(tutorial[s] == t);
			info(" %s", name[s]);
			cst1 += cost[s][t];
			mem1++;
		}
		info("; cst=%d, memb=%d\n", cst1, mem1);
		cst += cst1;
		mem += mem1;
		assert(mem1 + capacity[t] == maximum[t]);
	}
	info("total cost = %d, members = %d\n", cst, mem);
	info("--------------------\n");

	return cst;
}
*/


/* This actually performs the relocation of a student, and updates all the
   data structures.  */

void move(int const s, int const tb)
{
	int ta = tutorial[s];

	assert(ta != tb);
	assert(0 <= ta && ta < tutors && 0 <= tb && tb < tutors);
	assert(0 <= s);
	assert(s < students);
	assert(head[ta] >= 0);

	tutorial[s] = tb;

	int j = s;
	while (prev[j] >= 0) j = prev[j];
	assert(head[ta] == j);

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



int *seen = NULL;
int *delta = NULL;
int cycle, gain;



int push(int ta, int d) {

	if (seen[ta]) {
		if (d < delta[ta]) {
			gain = d - delta[ta];
			info("cycle %d: %s", gain, slot[ta]);
			cycle = ta;
			return 1;
		}
		return 0;
	}

	if (capacity[ta] > 0) {
		if (d < 0) {
			gain = d;
			info("path %d: %s", gain, slot[ta]);
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

		int w = -1, wc = 0, sc = 0;
		forMember(s,ta) {
			sc = cost[s][tb] - cost[s][ta];
			if (w < 0 || sc < wc) {
				w = s;
				wc = sc;
			}
		}
		if (w < 0) continue;

		if (push(tb, d + wc)) {
			if (cycle < 0) { /* we are on a path */
				if (d < 0) { /* we are on the useful part */
					info(" «%s:%d« %s", name[w], wc, slot[ta]);
					move(w, tb);
					seen[ta] = 0;
					return 1;
				}
				info(" end\n");
				/* path up to here is useless */
			} else { /* we are on a cycle */
				info(" «%s:%d« %s", name[w], wc, slot[ta]);
				move(w, tb);
				if (ta != cycle) { /* this is not the start */
					seen[ta] = 0;
					return 1;
				}
				info(" close\n");
				/* just closed a cycle */
				//break;
			}
		}

	}

	seen[ta] = 0;
	return 0;
}



int assign(void)
{
	gain = 0;
	forTutor(t)
		seen[t] = 0;

	forTutor(ta) {
		seen[ta] = 1;
		delta[ta] = 0;
		forTutor(tb) {
			if (ta == tb) continue;

			int w = -1, wc = 0, sc = 0;
			forMember(s,ta) {
				sc = cost[s][tb] - cost[s][ta];
				if (w < 0 || sc < wc) {
					w = s;
					wc = sc;
				}
			}
			if (w < 0) continue;


			if (wc < 0) {
				if (push(tb, wc)) {
					info(" «%s:%d« %s top\n", name[w], wc, slot[ta]);
					move(w, tb);
					//break;
				}
			}
		}
		seen[ta] = 0;
	}

	return 0;
}



void readCliArguments(char **argv)
{
	slot = malloc((size_t)tutors * sizeof(char *));
	assert(slot);
	maximum = malloc((size_t)tutors * sizeof(int));
	assert(maximum);
	places = 0;

	forTutor(t) {
		slot[t] = argv[t+1];
		char *col = strchr(slot[t], '=');
		if (!col) {
			if (t > 0) maximum[t] = maximum[t-1];
			else errx(1, "Missing `=` in argument %s", slot[t]);
		} else {
			char *endptr;
			maximum[t] = (int)strtol(col+1, &endptr, 10);
			if (*endptr) errx(1, "Cannot parse integer");
			*col = '\0';
		}
		places += maximum[t];
	}
}



void statistics(void)
{
	int costMin = cost[0][0], costMax = 0, costSum = 0,
		offCount[tutors], offSum = 0;
	double costAvg = 0, offAvg = 0;

	forTutor(o) offCount[o] = 0;

	forTutor(t) forMember(s,t) {
		int c = cost[s][t], o = offset[s][t];
		offCount[o]++;
		costSum += c;
		offSum += o;
		if (c > costMax) costMax = c;
		if (c < costMin) costMin = c;
	}

	costAvg = (double)costSum / students;
	offAvg = (double)offSum / students;
	double costDev = 0, offDev = 0;
	forTutor(t) forMember(s,t) {
		costDev += pow(costAvg - cost[s][t], 2);
		offDev += pow(offAvg - offset[s][t], 2);
	}
	costDev = sqrt(costDev / students);
	offDev = sqrt(offDev / students);

	info("%sOffset:  hist = %s[%d", fnBlue, fnRed, offCount[0]);
	for (int i = 1; i < tutors; i++) info(",%d", offCount[i]);
	info("]%s,  Σ = %s%d%s,  ⌀ = %.3f,  σ = %.3f\n", fnBlue, fnRed,
	        offSum, fnBlue, offAvg, offDev);
	info("Cost:  range = %d..%d,  Σ = %d,  ⌀ = %.3f,  σ = %.3f%s\n",
	        costMin, costMax, costSum, costAvg, costDev, fnNorm);
}



int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("%s\n",
#include "help.inc"
		       );
		return 0;
	}

	if (!isatty(fileno(stderr))) fnBlue = fnRed = fnNorm = "";

	tutors = argc - 1;

	readCliArguments(argv);
	readVotes();

	if (students < 1) errx(1, "No students to assign");

	calcOffsetCost();
	initialAssignment();
	statistics();

	seen = malloc((size_t)tutors * sizeof(int));
	assert(seen);
	delta = malloc((size_t)tutors * sizeof(int));
	assert(delta);
	do assign(); while (gain < 0);

	statistics();

	printf("# Initial capacities:");
	forTutor(t) printf(" %s=%d", slot[t], maximum[t]);
	printf("\n# WHO\tWHERE\t#N COST\t (VOTE:COST)*\n");
	forStudent(s) {
		int t = tutorial[s];
		printf("%s\t%s\t#%d %d\t", name[s], slot[t], s, cost[s][t]);
		forTutor(n) {
			int v = vote[s][n];
			if (v > -1) printf(" %s:%d", slot[v], cost[s][v]);
		}
		printf("\n");
	}
	printf("# Remaining capacities:");
	forTutor(t) printf(" %s=%d", slot[t], capacity[t]);
	printf("\n");

	return 0;
}
