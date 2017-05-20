#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "../Person.h"
#include "../ContactBook.h"
#include "../ContactTree.h"

int main() {
    ContactTree* root = createBook();
    int size = treeSize(root);

    int av = 1 / size;

    printf("Everythings ok\n");
}
