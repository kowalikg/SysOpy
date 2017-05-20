CC = gcc

all: compile clean

compile: mainTest.c
	cp ../libstatic.a .
	$(CC) $^ -g -o -Wall -L. -lstatic -o $@.out -std=gnu99
	./$@.out >> res.txt
clean:
	rm -f *.out *.so *.a
