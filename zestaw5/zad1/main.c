#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void parseLine(char *line);

void launchProgram(char *arguments[], int pipefd[2], bool last);
int main() {

    printf(">>");
    size_t buffer_size = 32;
    char *line = calloc(buffer_size, sizeof(char));
    getline(&line, &buffer_size, stdin);
    parseLine(line);
    return 0;
}

void parseLine(char *line) {

#define MAX_PROGRAMS 20
#define MAX_ARGUMENTS 4

    char* programs[MAX_PROGRAMS];

    char* arguments[MAX_PROGRAMS][MAX_ARGUMENTS];

    int programsCounter = 0;
    int argumentsCounter = 0;

    char * singleProgram;
    char * argument;

    singleProgram = strtok(line, "|\n");
    programs[programsCounter++] = singleProgram;

    while(programsCounter < MAX_PROGRAMS){ // wyodrebnimy programy
        singleProgram = strtok(NULL, "|\n");

        if (!singleProgram) break;
        programs[programsCounter++] = singleProgram;

    }

    for (int i = 0 ; i < programsCounter; i++) { // wyodrebniamy programy + argumenty
        argumentsCounter = 0;
        singleProgram = strtok(programs[i], " \t\n");
        arguments[i][argumentsCounter++] = singleProgram;


        do{
            argument = strtok(NULL," \n\t");
            arguments[i][argumentsCounter++] = argument;

        }
        while(argumentsCounter < MAX_ARGUMENTS && argument != NULL);
    }

    int pipefd[2]; //tablica do odczytu potokow
    pipe(pipefd); // twrorzymy potok
    // pipefd[0] = mozemy odczytywac dane
    // pipefd[1] = mozemy zapisywac dane


    for (int i = 0; i < programsCounter ; i++){
        pipe(pipefd); // aktualizujemy potok
        //odpalamy program do przekazania dalej
        launchProgram(arguments, pipefd, false);
    }
    // ostatni program odpalamy bez aktualizacji tak aby nam wyrzucil standardowe wyjscie a nie zapisywal
    launchProgram(arguments[programsCounter -1], pipefd, true);
    close(pipefd[0]); // zamykamy odczyt

}

void launchProgram(char *arguments[], int pipefd[2], bool last) {

    pid_t forked = fork();

    if (forked == -1){ // blad forka
        fprintf(stderr, "Cannot fork\n");
        exit(1);
    }
    else if (forked == 0){
        if(pipefd[0] != -1) { // jezeli da sie cos odczytac to
            dup2(pipefd[0], STDIN_FILENO); // odczytaj z pipefd[0] i wrzuc na wejscie programu
        }
        if(!last) { // jezeli to nie jest ostatni program to zapisz wyjscie do pipefd[1]
            dup2(pipefd[1], STDOUT_FILENO);
        }
        if(execvp(arguments[0],arguments) == -1 && execv(arguments[0],arguments) == -1){
            fprintf(stderr, "Cannot launch program: %s\n", arguments[0]);
            exit(1);
        }

        close(pipefd[1]); // zamykamy zapis

    }
    else {
        close(pipefd[1]); // zamykamy zapis
        dup2(pipefd[0], STDIN_FILENO); // wrzuc dane poczatkowe na wejscie
        wait(NULL); // poczekaj az zakoncza sie wszystkie dzieciaki
    }

}