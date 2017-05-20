#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


bool parseArguments(int argc, char **argv);
bool isNumber(char *string);

int main(int argc, char** argv) {

    if (!parseArguments(argc, argv)){
        fprintf(stderr, "Invalid arguments.\n");
        exit(1);
    }
    printf("Generating file...\n");
    char * fileName = argv[1];
    int size = atoi(argv[2]);

    printf("Filename: %s\nSize: %d\n", fileName, size);

    FILE * file = fopen(fileName, "w"); // tylko do odczytu

    for (int i = 0; i < size; i++){
        for (int j = 0; j < sizeof(int); j++){
            fprintf(file, "%d", rand() % 10);
        }
        fprintf(file, ":");
        for (int j = 0; j < 1024 - sizeof(int) - 2; j++){
            char randomChar = rand() % 26 + 'a';
            fprintf(file, "%c", randomChar);
        }
        fprintf(file, "\n");
    }

    return 0;
}

bool parseArguments(int argc, char **argv) {
    if (argc != 3) return false;
    if (!isNumber(argv[2])) return false;
    return true;
}

bool isNumber(char *string) {
    int i = 0;
    while(string[i] != '\0'){
        if (string[i] >= '0' && string[i] <= '9') i++;
        else return false;
    }
    return true;
}
