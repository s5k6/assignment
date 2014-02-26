cflags = -std=c99 -O6 -Wall -Wextra -Wpedantic -Wbad-function-cast \
	-Wconversion -Wwrite-strings -Wstrict-prototypes -Werror

.PHONY : all clean run

all : assi

clean :
	rm -f assi *.o *.inc *.dat *.log

run : assi
	./mkdata biased | shuf | tee in.log | ./assi a=20,b,c,d,e >| out.log

assi : main.o
	gcc -o $@ $^ -lm
	strip $@

%.o : %.c
	gcc $(cflags) -c -o $@ $<

main.o : help.inc

help.inc : help.txt

%.inc : %.txt
	sed 's/\\/\\\\/g;s/"/\\"/g;s/^/"/;s/$$/\\n"/' < $< > $@
