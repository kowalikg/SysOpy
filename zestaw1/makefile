CC = gcc
OBJECTS = Person.o ContactBook.o ContactTree.o
all: libstatic libshared

libstatic: Person.c ContactBook.c ContactTree.c
	$(CC) -c $^
	ar rcs $@.a $(OBJECTS)

libshared: Person.c ContactBook.c ContactTree.c
	$(CC) -c -fpic $^
	$(CC) -shared -fPIC -o $@.so $(OBJECTS)


