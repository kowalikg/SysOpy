#include "Person.h"
#include <stdio.h>
#include <stdlib.h>


Person* newPerson(int id, char* name){
    Person* p = (Person*) malloc(sizeof(Person));
    p->id = id;
    p->name = name;

    return p;
}
void displayPerson(Person* p){
    if(p != NULL)
    printf("ID: %d, name: %s\n",p->id, p->name);
}