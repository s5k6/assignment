
#ifndef parser_h_r7MTSxC8Pyur
#define parser_h_r7MTSxC8Pyur

typedef struct {
	int students;
	int tutors;
	char const *const *name;
	char const *const *slot;
	int *const *vote;
	int *const *cost;
	int const *capacity;
} fixed;


fixed const *readVotes(int const n, char const * const slot[]);

#endif
