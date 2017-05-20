//
// Created by gaba on 3/15/17.
//

#include "ContactTree.h"
#include "Person.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

ContactTree* createTree(){
    ContactTree * root =  (ContactTree*)malloc(sizeof(ContactTree));
    root->person = NULL;
    root->parent = NULL;
    root->left = NULL;
    root->right = NULL;

    return root;
}
ContactTree* pushPersonByName(ContactTree *root, Person *p){
    if(root->person == NULL){
        root->person = p;
        return root;
    }
    ContactTree * c =  (ContactTree*)malloc(sizeof(ContactTree));
    c->person = p;
    c->parent = NULL;
    c->left = NULL;
    c->right = NULL;

    ContactTree * x = root;
    ContactTree * prev = root->parent;

    while(x != NULL){
        if(!isLater(x->person->name, c->person->name)){
            prev = x;
            x = x->right;
        }
        else{
            prev = x;
            x = x->left;
        }

    }
    if(!isLater(prev->person->name, c->person->name)) {
        prev->right = c;
        c->parent = prev;
    }
    else{
        prev->left = c;
        c->parent = prev;
    }
    return root;




}
ContactTree * maxName(ContactTree * root){
    ContactTree * c = root;

    while(c->right != NULL) c = c->right;

    return c;
}

ContactTree * minName(ContactTree * root){
    ContactTree * c = root;

    while(c->left != NULL) c = c->left;

    return c;
}
ContactTree* successor(ContactTree* c){

    if(c->right != NULL)
        return minName(c->right);

    ContactTree * x;

    do {
        x = c;
        c = c->parent;
    }
    while(c != NULL && c->left != x);

    return c;
}
void displayTree(ContactTree* root){
    if (root == NULL) return;
    printf("Person:\n");
    if(root->person != NULL)
        displayPerson(root->person);

    if(root->right != NULL) displayTree(root->right);
    if(root->left != NULL) displayTree(root->left);

}
ContactTree* findContactTree(ContactTree * root, char* name){
    if (root->person == NULL) return NULL;

    ContactTree * x = root;

    while(x != NULL){
        if (x->person-> name == name) return x;
        if(strcmp(x->person->name, name) < 0) x = x->right;
        else x = x->left;

    }
    return NULL;
}
ContactTree * deleteTree(ContactTree* root){

    if(root->left != NULL) deleteTree(root->left);
    if(root->right != NULL) deleteTree(root->right);
    free(root->person);
    free(root);
    root = NULL;
    return root;
}
ContactTree* deleteContactTree(ContactTree* c, bool toFree){
    if (c->left == NULL && c->right == NULL){
        if(isLater(c->parent->person->name, c->person->name)){
            c->parent->left = NULL;

        }
        else{
            c->parent->right= NULL;
        }
        if(toFree){
            free(c->person);
            free(c);
        }
    }
    else if (c->left == NULL && c->right != NULL){
        if(isLater(c->parent->person->name, c->person->name)){
            c->parent->left = c->right;

        }
        else{
            c->parent->right = c->right;
        }
        if(toFree){
            free(c->person);
            free(c);
        }
    }
    else if (c->left != NULL && c->right == NULL){
        if(isLater(c->parent->person->name, c->person->name)){
            c->parent->left = c->left;

        }
        else{
            c->parent->right = c->left;
        }
        if(toFree){
            free(c->person);
            free(c);
        }
    }
    else{

        ContactTree * x = deleteContactTree(successor(c), false);

        c->person->name = x->person->name;
        c->person->id = x->person->id;
        if(toFree){
            free(x->person);
            free(x);
        }
    }
    return c;
}

bool isLater(char* a, char* b){
    if(strcmp(a, b) < 0){
        return false;
    }
    return true;
}

int treeSize(ContactTree* root){
    if(root != NULL && root->person != NULL){
        return treeSize(root->left) + treeSize(root->right) + 1;
    }
    return 0;
}
ContactTree * sortByID(ContactTree * root) {
    ContactTree *sorted = createTree();

    pushRec(sorted, root, true);
    deleteTree(root);
    return sorted;
}
ContactTree * sortByName(ContactTree * root) {
    ContactTree *sorted = createTree();

    pushRec(sorted, root, false);
    deleteTree(root);
    return sorted;
}
ContactTree* pushRec(ContactTree* sorted, ContactTree * root, bool byID){
    if (root != NULL){
        if (byID){
            sorted = pushPersonByID(sorted, newPerson(root->person->id, root->person->name));
        }
        else{
            sorted = pushPersonByName(sorted, newPerson(root->person->id, root->person->name));
        }

        pushRec(sorted, root->left, byID);
        pushRec(sorted, root->right, byID);
    }
}
ContactTree* pushPersonByID(ContactTree* root, Person *p){
    if(root->person == NULL){
        root->person = p;
        return root;
    }
    ContactTree * c =  (ContactTree*)malloc(sizeof(ContactTree));
    c->person = p;
    c->parent = NULL;
    c->left = NULL;
    c->right = NULL;

    ContactTree * x = root;
    ContactTree * prev = root->parent;

    while(x != NULL){
        if(c->person->id > x->person->id){
            prev = x;
            x = x->right;
        }
        else{
            prev = x;
            x = x->left;
        }

    }
    if(c->person->id > prev->person->id) {
        prev->right = c;
        c->parent = prev;
    }
    else{
        prev->left = c;
        c->parent = prev;
    }
    return root;


}