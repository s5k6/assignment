cflags = -std=c99 -g -Wall -Wextra -Wpedantic -Wbad-function-cast \
	-Wconversion -Wwrite-strings -Wstrict-prototypes -Werror

.PHONY : all clean test-parser run

all : main

clean :
	rm -f a.out main a.out *.o *.dat

run : main test-90-5.dat
	./main A=20 B=20 C=20 D=20 E=20 <test-90-5.dat

test-90-5.dat : mkdata
	./mkdata biased >$@

main : main.o
	gcc -o $@ $^ -lm

%.o : %.c
	gcc $(cflags) -c -o $@ $<
