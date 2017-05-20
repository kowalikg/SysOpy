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
#include <signal.h>

int linesToRead;
char * word;
int threadsNumber;
int signalToSend;
int option;

int file;
int delay = 1;
pthread_t* threadArray;

bool parseArgument(int argc, char **argv);
bool isANumber(char *string);

void signalHandler(int signal);
void openFile(char *name);
void generateThreads(int number);
static void* start_routine();
void cancelThreads();

void sendSignals();

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
    option = atoi(argv[5]);
    signalToSend = atoi(argv[6]);

    switch(option){
        case 2:
            signal(signalToSend, SIG_IGN);
            break;
        case 3:
            signal(signalToSend, signalHandler);
            break;
        default: break;
    }

    openFile(fileName);
    generateThreads(threadsNumber);

    return 0;
}
void signalHandler(int signal){
    printf("Captured signal %d in thread %lu, pid %d\n", signal, pthread_self(), getpid());
}
static void* start_routine(){
    switch(option){
        case 4:
            signal(signalToSend, SIG_IGN);
            break;
        case 3:
        case 5:
            signal(signalToSend, signalHandler);
            break;
        default: break;
    }
    while(delay);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL); // wlacz przechwtywanie
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); // asynchroniczne

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
            cancelThreads();
            return (void*) 0;
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
    for (int i = 0; i < number; i++){
        if (pthread_create(&threadArray[i], NULL, start_routine, NULL) != 0) { // uchwyt, atrybuty, wskaznik do funkcji, argument do funkcji
            fprintf(stderr, "Cannot create thread: %d\n", i);
            exit(1);
        }
    }
    delay = 0;
    sendSignals();
    for (int i = 0; i < number; i++){
        if (pthread_join(threadArray[i], NULL) != 0){ // uchwyt, dokad retval
            fprintf(stderr, "Cannot join thread: %d\n", i);
            exit(1);
        }
    }

}

void sendSignals() {
    switch(option){
        case 1:
        case 2:
        case 3:
            printf("Sending signal %d to main process\n", signalToSend);
            kill(getpid(), signalToSend);
            break;
        case 4:
        case 5:
            printf("Sending signal %d thread %lu\n", signalToSend, threadArray[0]);
            pthread_kill(threadArray[0], signalToSend);
            break;
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
    if (argc != 7) return false;

    if (argv[1] == NULL || !isANumber(argv[1])) return false;
    if (argv[3] == NULL || !isANumber(argv[3])) return false;
    if (argv[5] == NULL || !isANumber(argv[5])) return false;
    if (argv[6] == NULL || !isANumber(argv[5])) return false;

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
