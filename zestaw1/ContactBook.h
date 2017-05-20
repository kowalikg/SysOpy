//
// Created by gaba on 3/14/17.
//
#include "Person.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifndef SYSOPY_CONTACTBOOK_H
#define SYSOPY_CONTACTBOOK_H

typedef struct Contact{
    Person* person;
    struct Contact* next;
    struct Contact* prev;
} Contact;

Contact* createBook(void);
Contact* deleteBook(Contact *head);
Contact *pushPerson(Contact* head, Person* p);
void displayContact(Contact* c);
void displayBook(Contact* head);
Contact* deleteContact(Contact* head, Contact* c);
void displayTail(Contact * tail);
Contact* deleteTail(Contact* head);
Contact* findPerson(Contact* head, char* name);
Contact* findMax(Contact * head);
Contact* sort(Contact * head);
bool isEmpty(Contact * head);
#endif //SYSOPY_CONTACTBOOK_H
