#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

pid_t forked; // rozwidlacz

int type;

union sigval signalValue;

bool parseArguments(int argc, char **argv);

bool isNumber(char *string);

void function(int sig);

int main(int argc, char *argv[]) {
    if (!parseArguments(argc,argv)){
        fprintf(stderr, "Wrong arguments\n");
        exit(1);
    }

    int L = atoi(argv[1]);
    type = atoi(argv[2]);

    struct sigaction signalAction;
    signalAction.sa_handler = function;
    sigemptyset(&signalAction.sa_mask);

    sigaction(SIGRTMIN + 1, &signalAction, NULL);
    sigaction(SIGRTMIN + 2, &signalAction, NULL);
    sigaction(SIGINT, &signalAction, NULL);
    sigaction(SIGUSR1, &signalAction, NULL);
    sigaction(SIGUSR2, &signalAction, NULL);


    forked = fork(); // rozwidlenie

    if (forked == -1) { // blad rozwidlenia
        fprintf(stderr, "Cannot fork process\n");
        exit(1);

    }
    else if (forked == 0) {  // jak dzieckiem -< tu umieszcamy co sie ma robic dziecko, a dziecko ma swoja funkcje specjalna do tego
        while(1)
            pause(); //chodz sobie dopoki cie nie wyslemy Ci sygnalu przerwania

    }
    else {
        int i = 0;
        // jak rodzicem to robimy to co trzeba
        switch (type) {
            case 1:
                printf("Parent: sending %d SIGUSR1 signals\n", L);
                while(i < L){
                    kill(forked, SIGUSR1); // wysylamy L sygnalow SIGUSR1 fo forked

                    i++;
                    sleep(1);
                }
                kill(forked, SIGUSR2); // sygnal SIGUSR2
                break;
            case 2:
                // unia przechowuje sival_int albo sival_ptr (void*)

                signalValue.sival_int = 0;

                printf("Parent: sending %d SIGUSR1 signals\n", L);

                while(i < L){
                    sigqueue(forked, SIGUSR1, signalValue); // sigqueue przyjmuje unie jako argument

                    i++;
                    sleep(1);
                }
                sigqueue(forked, SIGUSR2, signalValue); // wysylamy sygnalek
                break;
            case 3:
                printf("Parent: sending %d SIGURTMIN + 1 signals\n", L);
                while(i < L){
                    kill(forked, (SIGRTMIN + 1)); // wysylamy L sygnalow SIGUSR1 fo forked

                    i++;
                    sleep(1);
                }
                kill(forked, SIGRTMIN + 2); // sygnal SIGUSR2
                break;
        }
        int status;
        wait(&status); // czekamy na zakonczenie biezacego procesu (aby sie nie wyrzucil od razu)
        printf("%d: program exited with status %d\n", getpid(), status);

	return 0;
    }
}

void function(int sig) {
    // tutaj odbieramy sygnaly otrzymane przez bobasa

    signalValue.sival_int = 0;
    if (sig == SIGUSR1) {
        if (forked == 0) {
            printf("Child: receiving SIGUSR1\n");
            if (type == 2)
                sigqueue(getppid(), SIGUSR1, signalValue);
            else
                kill(getppid(), SIGUSR1);
        } else {
            printf("Parent: receiving SIGUSR1\n");
        }
    }
    else if (sig == SIGUSR2) {
        printf("Child: receiving SIGUSR2\n");
        exit(0);
    }
    else if (sig == SIGRTMIN + 1){
        if (forked == 0) {
            printf("Child: receiving SIGRTMIN + 1\n");
            kill(getppid(), SIGRTMIN + 1);
        } else {
            printf("Parent: receiving SIGRTMIN + 1\n");
        }
    }
    else if (sig == SIGRTMIN + 2){
        printf("Child: receiving SIGRTMIN + 2\n");
        exit(0);
    }
    else if (sig == SIGINT){
        if(kill(forked, SIGINT) == -1)
            fprintf(stderr, "Child: problem with killing\n");
        exit(0);
    }
}


bool parseArguments(int argc, char **argv) {
    if(argc != 3) return false;
    if(!isNumber(argv[1])) return false;
    if (strcmp(argv[2], "1") == 0 || strcmp(argv[2], "2") == 0 || strcmp(argv[2], "3") == 0)  return true;
    return false;
}

bool isNumber(char *string) {
    int i = 0;
    while(string[i] != '\0'){
        if (string[i] >= '0' && string[i] <= '9') i++;
        else return false;
    }
    return true;
}
