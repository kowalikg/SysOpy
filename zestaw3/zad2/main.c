#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>

bool parseArguments(int argc, char** argv);

void readFile(FILE *file, int seconds, int size);

void setEnviorementVariable(char *line);

void setVariable(char *variable, char *value);

void deleteVariable(char *variable);

void launchProgram(char *line, int seconds, int size);

bool isNumber(char *string);

int stringToNumber(char *string);

float convertToSec(struct timeval time);

int main(int argc, char *argv[]) {

    if(!parseArguments(argc, argv)){
        fprintf(stderr, "Invalid arguments!\n");
        exit(1);
    }
    FILE * file = fopen(argv[1], "r+");
    int seconds = stringToNumber(argv[2]);
    int size = stringToNumber(argv[3]);

    if(!file){
        fprintf(stderr, "Cannot open selected file: %s\n", argv[1]);
        exit(1);
    }

    readFile(file, seconds, size);
    fclose(file);
    return 0;
}

void readFile(FILE *file, int seconds, int size) {

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    while (nread = getline(&line, &len, file) != -1) {
        if (nread != 0){
            if (line[0] == '#') setEnviorementVariable(line);
            else launchProgram(line, seconds, size);

        }
    }
}

void launchProgram(char *line, int seconds, int size) {
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
        //struktury na limit
        rlim_t singleCpuLimit = (rlim_t) seconds;
        rlim_t singleMemoryLimit = (rlim_t) size;

        struct rlimit mainCpuLimit, mainMemoryLimit;

        getrlimit(RLIMIT_CPU, &mainCpuLimit);
        getrlimit(RLIMIT_AS, &mainMemoryLimit);

        if (mainCpuLimit.rlim_max < singleCpuLimit){
            fprintf(stderr, "Error: CPU limit shoud be downsized\n"); //teoretycznie nieroot moze tylko zmniejszac limity
            exit(-1);
        }
        else {
            mainCpuLimit.rlim_max = singleCpuLimit; //max
            mainCpuLimit.rlim_cur = 1; //soft

            if(setrlimit(RLIMIT_CPU, &mainCpuLimit) != 0){
                fprintf(stderr, "Error: cannot set CPU limit\n");
                exit(-1);
            }
        }

        if(mainMemoryLimit.rlim_max < singleMemoryLimit) {
            fprintf(stderr, "Error: memory limit shoud be downsized\n"); //teoretycznie nieroot moze tylko zmniejszac limity
            exit(-1);
        }
        else{
            mainMemoryLimit.rlim_cur = singleMemoryLimit * 1024 * 768;
            mainMemoryLimit.rlim_max = singleMemoryLimit * 1024 * 1024;
            if(setrlimit(RLIMIT_AS, &mainMemoryLimit) != 0) {
                fprintf(stderr, "Error: cannot set memory limit\n");
                exit(-1);
            }
        }

        if(execvp(program,arguments) == -1 && execv(program,arguments) == -1){ //jezeli sie nie powiedzie to zwroci -1, jezeli powiedzie to nic nie zwroci :/
            //excevp : arguments jako tablica i p-> szukamy sciezek w path
            //excev : jezeli podana zostanie sciazka bezwzgledna
            fprintf(stderr,"\nError: cannot run program %s\n", program);
            exit(-1);
        }
    }
    else if(pid > 0){ // rodzic
        int status;
        struct rusage programUsage;
        wait3(&status, 0, &programUsage); // czekamy na wykonanie potomnego

        if(WIFEXITED(status)){
            if(WEXITSTATUS(status) == 0){
                printf("\nProgram with pid %d exited succesfully: systime: %f, usertime:%f\n", pid, convertToSec(programUsage.ru_stime), convertToSec(programUsage.ru_utime));

            }
            else{
                printf("\nError: Program with pid %d exited with error: systime: %f, usertime:%f\n", pid, convertToSec(programUsage.ru_stime), convertToSec(programUsage.ru_utime));
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

bool parseArguments(int argc, char** argv) {
    if(argc != 4)
        return false;

    if(!isNumber(argv[2])) return false;
    if(!isNumber(argv[3])) return false;
    return true;

}
int stringToNumber(char *string) {
    int number = 0;
    int i = 0;

    while(string[i] != '\0'){
        number = 10 * number + (string[i] - '0');
        i++;
    }
    return number;
}

bool isNumber(char *string) {

    int i = 0;
    while(string[i] != '\0'){
        if (string[i] >= '0' && string[i] <= '9') i++;
        else return false;
    }
    return true;
}
float convertToSec(struct timeval time){
    return time.tv_sec + time.tv_usec / 1000000.0;
}