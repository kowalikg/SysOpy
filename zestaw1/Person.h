#ifndef BOOK_LIBRARY_H
#define BOOK_LIBRARY_H

typedef struct {
    int id;
    char* name;
} Person;

Person* newPerson(int id, char* name);
void displayPerson(Person* p);


#endif