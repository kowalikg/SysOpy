CC=gcc
OPT = -O1
all: lstatic lshared ldynamic clean
lstatic: main.c
	cp ../libstatic.a .
	$(CC) $^ $(OPT) -Wall -L. -$@ -o $@.out -std=gnu99
	echo $(OPT) >> res.txt
	echo "\nStatic \n" >> res.txt
	./$@.out >> res.txt

lshared: main.c
	cp ../libshared.so .
	$(CC) $^ -Wall -L. -Wl,-rpath=. -$@ -o $@.out -std=gnu99
	echo $(OPT) >> res.txt
	echo "\nShared\n" >> res.txt
	./$@.out >> res.txt

ldynamic: main_dynamic.c
	cp ../libshared.so .
	$(CC) $^ -Wall -o $@.out -ldl
	echo $(OPT) >> res.txt
	echo "\nDynamic\n" >> res.txt
	./$@.out >> res.txt
clean:
	rm -f *.out *.so *.a

	

