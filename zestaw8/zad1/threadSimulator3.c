//
// Created by gaba on 5/16/17.
//

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int linesToRead;
char * word;
int threadsNumber;

int file;
pthread_t* threadArray;
pthread_attr_t attributes;

bool parseArgument(int argc, char **argv);
bool isANumber(char *string);

void openFile(char *name);
void generateThreads(int number);
void* start_routine();

void cancelThreads();

int main(int argc, char** argv) {
    if (!parseArgument(argc, argv)){
        fprintf(stderr, "Invalid arguments.\n");
        exit(1);
    }

    threadsNumber = atoi(argv[1]);
    char * fileName = argv[2];
    int lines = atoi(argv[3]);
    linesToRead = lines;
    word = argv[4];

    openFile(fileName);
    generateThreads(threadsNumber);

    return 0;
}

void* start_routine(){
    char ** lines = malloc(linesToRead);

    for (int i = 0; i < linesToRead; i++) {
        lines[i] = malloc(1024 * sizeof(char));
    }
    for (int i = 0; i < linesToRead; i++) {
        if (read(file, lines[i], 1024) == -1) {
            fprintf(stderr, "Cannot read file\n");
            exit(1);
        }
    }

    for (int i = 0; i < linesToRead; i++) {
        if (strstr(lines[i], word) != NULL){
            printf(lines[i]);
            printf("Word %s found in line %s by %li\n", word, strtok(lines[i], ":"), pthread_self());
        }
    }
    return (void*) 0;

}
void cancelThreads() {
    for (int i = 0; i < threadsNumber; i++){
        if(threadArray[i] != pthread_self()){
            if (pthread_cancel(threadArray[i]) == -1){
                fprintf(stderr, "Cannot cancel thread\n");
                exit(1);
            }
        }
    }
}

void generateThreads(int number) {
    threadArray = malloc(sizeof(pthread_t) * number);

    pthread_attr_init(&attributes);
    pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);

    for (int i = 0; i < number; i++){
        if (pthread_create(&threadArray[i], &attributes, start_routine, NULL) != 0) { // uchwyt, atrybuty, wskaznik do funkcji, argument do funkcji
            fprintf(stderr, "Cannot create thread: %d\n", i);
            exit(1);
        }
        sleep(1);
    }

}

void openFile(char *name) {
    file = open(name, O_RDONLY);

    if (file == NULL) {
        fprintf(stderr, "Cannot open file\n");
        exit(1);
    }
}


bool parseArgument(int argc, char **argv) {
    if (argc != 5) return false;

    if (argv[1] == NULL || !isANumber(argv[1])) return false;
    if (argv[3] == NULL || !isANumber(argv[3])) return false;

    return true;

}
bool isANumber(char *string) {
    int i = 0;
    while(string[i] != '\0'){
        if (string[i] >= '0' && string[i] <= '9') i++;
        else return false;
    }
    return true;
}