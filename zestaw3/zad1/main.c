#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

bool parseArguments(int argc);

void readFile(FILE *file);

void setEnviorementVariable(char *line);

void setVariable(char *variable, char *value);

void deleteVariable(char *variable);

void launchProgram(char *line);

int main(int argc, char *argv[]) {

    if(!parseArguments(argc)){
        fprintf(stderr, "Invalid arguments!\n");
        exit(1);
    }
    FILE * file = fopen(argv[1], "r+");

    if(!file){
        fprintf(stderr, "Cannot open selected\n");
        exit(1);
    }

    readFile(file);
    fclose(file);
    return 0;
}

void readFile(FILE *file) {

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    while (nread = getline(&line, &len, file) != -1) {
        if (nread != 0){
            if (line[0] == '#') setEnviorementVariable(line);
            else launchProgram(line);

        }
    }
}

void launchProgram(char *line) {
    char * program;

    const int MAX_ARGUMENTS = 6;
    char * arguments[MAX_ARGUMENTS];

    program = strtok(line, " \n\t");
    arguments[0] = program;

    int argumentsCount = 0;
    char *arg;

    while(argumentsCount < MAX_ARGUMENTS && arg != NULL){
        arg = strtok(NULL," \n\t");

        arguments[++argumentsCount] = arg;

    }

    pid_t pid = fork(); // rozwidlenie programu na proces macierzysty i potomny
    //uruchamiamy nowy proces od tej linijki
    //W procesie macierzystym funkcja zwróci numer PID procesu potomnego
    // W procesie potomnym funkcja zwróci wartość 0

    if(pid == 0) { //proces potomny, odpalamy program
        if(execvp(program,arguments) == -1 && execv(program,arguments) == -1){ //jezeli sie nie powiedzie to zwroci -1, jezeli powiedzie to nic nie zwroci :/
            //excevp : arguments jako tablica i p-> szukamy sciezek w path
            //excev : jezeli podana zostanie sciazka bezwzgledna
            fprintf(stderr,"\nError: cannot run program %s\n", program);
            exit(-1);
        }
    }
    else if(pid > 0){ // rodzic
        int status;
        waitpid(pid, &status, 0); // czekamy na wykonanie potomnego

        if(WIFEXITED(status)){
            if(WEXITSTATUS(status) == 0){
                printf("\nProgram with pid %d exited succesfully\n", pid);
            }
            else{
                printf("\nError: Program with pid %d exited with error\n", pid);
                exit(-1);
            }
        }
    }
    else{
        fprintf(stderr,"\nCould not create process for program %s\n",program);
        exit(-1);
    }

}

void setEnviorementVariable(char *line) {
    char * whichVariable;
    char * variableValue;

    whichVariable = strtok(++line," \n\t");
    variableValue = strtok(NULL," \n\t");

    variableValue != NULL ? setVariable(whichVariable, variableValue) : deleteVariable(whichVariable);
}

void deleteVariable(char *variable) {
    if(unsetenv(variable))
        fprintf(stderr,"Error: Cannot delete variable %s\n",variable);
    else
        printf("Successfully deleted variable %s\n", variable);
}

void setVariable(char *variable, char *value) {
    if (setenv(variable, value, 1) != 0) {  // 0 - jezeli istnieje to nie ustawiaj, 1 - wymuszone ustawianie
        // setenv zwraca 0 jezeli sie powiodlo, -1 jak nie
        fprintf("Error: Cannot set variable %s to %s\n", variable, value);
    } else
        printf("Successfully setted variable %s to %s\n", variable, value);

}

bool parseArguments(int argc) {
    if(argc != 2)
        return false;
    return true;
}