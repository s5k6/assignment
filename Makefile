cflags = -std=c99 -g -Wall -Wextra -Wpedantic -Wbad-function-cast \
	-Wconversion -Wwrite-strings -Wstrict-prototypes -Werror

.PHONY : all clean test-parser run

all : main

clean :
	rm -f a.out main testParser a.out *.o *.dat

test-parser : testParser test-90-5.dat
	./testParser A B C D E <test-90-5.dat | cmp - test-90-5.dat

run : main test-90-5.dat
	./main A B C D E <test-90-5.dat

test-90-5.dat : mkdata
	./mkdata >$@


testParser : testParser.o parser.o
	gcc -o $@ $^

main : main.o parser.o
	gcc -o $@ $^

testParser.o : testParser.c parser.h
main.o : main.c parser.h

%.o : %.c
	gcc $(cflags) -c -o $@ $<
