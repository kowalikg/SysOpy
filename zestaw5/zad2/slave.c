#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <complex.h>
#include <unistd.h>

struct complexToSend{
    double re;
    double im;
    int it;
};

struct complexToSend generatePoint(int k);
bool parseArguments(int amount, char **arguments);
bool isNumber(char *string);
int generateIterationBreak(struct complexToSend result, int K);

int main(int argc, char *argv[]) {
    srand(time(NULL));

    if (!parseArguments(argc, argv)){
        fprintf(stderr, "Invalid arguments\n");
        exit(1);
    }

    char pipePath[100];
    strcpy(pipePath, argv[1]);

    int N = atoi(argv[2]);
    int K = atoi(argv[3]);

    char buffer[100];
    memset(buffer, 0, 100); // czyszcze buffor bo inaczej smieci wypisuje

    int pipe = open(pipePath, O_WRONLY); // otwieramy istniejacy potok

    if(pipe == -1){ //jezeli nie ma potoku
        fprintf(stderr, "Cannot open pipe\n");
        exit(1);
    }

    for (int i = 0; i < N; i++){
        struct complexToSend point = generatePoint(K);
        sprintf(buffer,"%f|%f|%d\n", point.re, point.im, point.it);
        write(pipe, buffer, 100); // zapis z buffora do potoku
    }

    close(pipe);
    return 0;
}

struct complexToSend generatePoint(int k) {
    struct complexToSend result;

    result.re = (double) ((rand() % 3000) - 2000) / 1000; // zakres: -2 < re < 1
    result.im = (double) ((rand() % 2000) - 1000) / 1000; // zakres -1 < im < 1
    result.it = generateIterationBreak(result, k);

    return result;
}

int generateIterationBreak(struct complexToSend result, int K) {
    double complex c = result.re + (result.im * I);
    double complex zn = 0;

    int iterations = 0;
    while (iterations < K && cabs(zn) < 2){
        // zamaist cabs re^2 + im ^2 < 4
        zn = cpow(zn, 2) + c;
        iterations++;
    }
    return iterations;
}

bool parseArguments(int amount, char **arguments) {
    if (amount != 4) {
        return false;
    }
    if (!isNumber(arguments[2])){
        return false;
    }
    if (!isNumber(arguments[3])){
        return false;
    }

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