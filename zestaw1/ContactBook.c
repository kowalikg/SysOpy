//
// Created by gaba on 3/14/17.
//

#include "ContactBook.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
Contact* createBook(){
    Contact *head = (Contact*)malloc(sizeof(Contact));
    head->next = NULL;
    head->prev = NULL;
    head->person = NULL;
    return head;
}

Contact* pushPerson(Contact* head, Person* p){

    if (head->person == NULL){
        head->person = p;
    }
    else{
        Contact *c = (Contact*)malloc(sizeof(Contact));

        c->person = p;
        c->prev = NULL;
        c->next = head;

        head->prev = c;

        head = c;
    }

    return head;

}

void displayTail(Contact * head){
    Contact *tail = head;

    while(tail->next != NULL){
        tail = tail->next;
    }

    while(tail != NULL){
        displayContact(tail);
        tail = tail->prev;
    }
}
void displayContact(Contact* c){
    displayPerson(c->person);
}
void displayBook(Contact *head){
    printf("Book: \n");
    Contact* c = head;
    while(c != NULL){
        displayContact(c);
        c = c->next;
    }
}
Contact* deleteContact(Contact* head, Contact *c){

    if (c == NULL) return NULL;

    Contact* tmp = c;

    Contact* prev = c->prev;
    if (prev != NULL) {
        prev->next = c->next;

        if(c->next != NULL){
            c->next->prev = prev;
        }

        free(tmp->person);
        free(tmp);
        tmp = NULL;
    }
    else{
        head = c->next;
        if (head!= NULL) head->prev = NULL;

        free(tmp->person);
        free(tmp);

    }
    return head;


}
Contact* deleteBook(Contact *head){
    while (head != NULL){
        head = deleteContact(head, head);
    }
    head = NULL;
    return head;



}
Contact* findPerson(Contact* head, char* name){

    if (head == NULL) return NULL;

    Contact* c = head;

    while(c != NULL){
        if(c->person->name == name){
            return c;
        }
        c = c->next;
    }
    return NULL;


}
Contact * findMax(Contact *head){
    Contact * c = NULL;
    char * max = "";

    while(head != NULL){
        if (strcmp(head->person->name, max) >= 0){
            max = head->person->name;
            c = head;
        }
        head = head->next;
    }


    return c;
}
Contact *sort(Contact * head){

    Contact* sorted = createBook();

    while(head != NULL){
        Contact * max = findMax(head);
        Person* p = newPerson(max->person->id, max->person->name);

        head = deleteContact(head, max);

        sorted = pushPerson(sorted, p);
    }

    return sorted;


}
Contact* deleteTail(Contact* head){
    Contact* x = head;
    while(x->next != NULL) x = x->next;

    return deleteContact(head, x);

}
bool isEmpty(Contact * head){
    if (head->person!=NULL) return false;
    return true;
}

