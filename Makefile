cflags = -std=c99 -g -Wall -Wextra -Wpedantic -Wbad-function-cast \
	-Wconversion -Wwrite-strings -Wstrict-prototypes


.PHONY : all clean dist-clean run

all : main

clean :
	rm -f a.out main data.inc

run : all
	./main

data.inc : mkdata
	./mkdata 75 5 20 >| $@

main : main.c data.inc
	gcc $(cflags) -o $@ $<
