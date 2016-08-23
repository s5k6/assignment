cflags = -std=c99 -O6 -Wall -Wextra -Wpedantic -Wbad-function-cast \
	-Wconversion -Wwrite-strings -Wstrict-prototypes -Werror

all : assi

clean :
	rm -f assi *.o *.inc

assi : main.o
	gcc -o $@ $^ -lm
	strip $@

%.o : %.c
	gcc $(cflags) -c -o $@ $<

main.o : help.inc

help.inc : help.txt

%.inc : %.txt
	sed 's/\\/\\\\/g;s/"/\\"/g;s/^/"/;s/$$/\\n"/' < $< > $@
