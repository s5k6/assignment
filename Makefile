cflags = -std=c99 -g -Wall -Wextra -Wpedantic -Wbad-function-cast \
	-Wconversion -Wwrite-strings -Wstrict-prototypes -Werror

.PHONY : all clean test-parser run

all : main

clean :
	rm -f a.out main a.out *.o *.dat

run : main test-90-5.dat
	./main A B C D E <test-90-5.dat

test-90-5.dat : mkdata
	./mkdata >$@

main : main.o
	gcc -o $@ $^

%.o : %.c
	gcc $(cflags) -c -o $@ $<
