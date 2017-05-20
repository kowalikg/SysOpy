//
// Created by gaba on 3/15/17.
//

#ifndef SYSOPY_CONTACTTREE_H
#define SYSOPY_CONTACTTREE_H
#include "Person.h"
#include <stdbool.h>
typedef struct ContactTree {
    Person * person;
    struct ContactTree * parent;
    struct ContactTree * left;
    struct ContactTree * right;
} ContactTree;

ContactTree* createTree(void);
ContactTree* deleteTree(ContactTree * root);
ContactTree* pushPersonTree(ContactTree* root, Person* p);
ContactTree* pushPersonByID(ContactTree* root, Person* p);
ContactTree* findContactTree(ContactTree* root, char* name);
ContactTree* deleteContactTree(ContactTree* c, bool toFree);
ContactTree* sortByName(ContactTree* root);
ContactTree* successor(ContactTree* c);
ContactTree * maxName(ContactTree * root);
ContactTree * minName(ContactTree * root);
ContactTree * sortByID(ContactTree * root);
ContactTree* pushRec(ContactTree* sorted, ContactTree * root, bool byID);
void displayTree(ContactTree* root);
bool isLater(char* a, char* b);
int treeSize(ContactTree* root);

#endif //SYSOPY_CONTACTTREE_H
