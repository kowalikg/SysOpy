//
// Created by gaba on 3/29/17.
//
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char *argv[]){
    if (argc != 2){
        printf("Error: wrong arguments\n");
        exit(-1);
    }
    char * variable = argv[1];
    char * value = getenv(variable);
    if(value){
        printf("Variable %s value: %s\n", variable, value);
    }
    else{
        printf("No variable %s exists\n", variable);
    }

}