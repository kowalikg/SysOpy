#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <wait.h>
#include <time.h>

struct child{
    bool ifAllowed;
    pid_t pid;
};

struct child * children;

volatile int requests = 0;

bool allowed = false;

int N = 0;

pid_t newPid;

bool parseArguments(int argc, char **argv);

bool isNumber(char *string);

void function(int sig, siginfo_t *siginfo, void *context);

int randomSignal();

void waitForRequest(int K);

void allowSending(int l);

int getIndexByPID(int pid);

int main(int argc, char *argv[]) {
    if (!parseArguments(argc, argv)){
        fprintf(stderr, "Invalid arguments\n");
        exit(1);
    }

    N = atoi(argv[1]);
    int K = atoi(argv[2]);

    struct sigaction signalAction;
    signalAction.sa_handler = function;
    sigemptyset(&signalAction.sa_mask);
    signalAction.sa_flags = SA_SIGINFO;

    //niech sygnal sigrtmin zastepuje sygnal sigusr1 (zgodnie z ogloszeniem na upel)

    for (int i = SIGRTMIN; i <= SIGRTMAX; i++){
        sigaction(i, &signalAction, NULL);
    }
    //sygnal zakonczenia
    sigaction(SIGINT, &signalAction, NULL);

    //tablica dzieci

    children = malloc(N * sizeof(struct child));

    for (int i = 0; i < N; i++){
        children[i].ifAllowed = false;
    }
    // tworzenie N procesow
    for (int i = 0 ; i < N ; i++){
        newPid = fork();

        if(newPid == -1){ //cos sie popsulo
            fprintf(stderr, "Problem with forking\n");
            exit(1);
        }
        else if (newPid == 0){ //bobas
            srand(getpid()); //punkt startowy generatora

            int sec = rand() % 11;

            printf("Lauching process %d: sleep for %d seconds\n", getpid(), sec);
            //spij
            sleep(sec);

            // wlacz timery
            time_t beginTime;
            time(&beginTime);

            // sygnal zezwolenia
            kill(getppid(), SIGRTMIN);

            // pause -> oczekiwanie na dowolny sygnal, w tym wypadku na SIGRTMAX
            pause();
            // jezeli otrzymamy to wysylamy randomowy
            int r = randomSignal();
            kill(getppid(), r);
            // pobieramy czas
            time_t endTime;
            time(&endTime);
            // exit status
            int exitStatus = (int) (endTime - beginTime);
            exit(exitStatus);

        }
        else{ //rodzic
            //zapisujemy numer procesu dziecka
            children[i].pid = newPid;
        }
    }

    waitForRequest(K);
    allowSending(N);

    for (int i = 0; i < N; i++) {
        int exitStatus;

        waitpid(children[i].pid, &exitStatus, 0);

        exitStatus = WEXITSTATUS(exitStatus);

        printf("%d: successfully closed with exit status %d\n", children[i].pid, exitStatus);
    }

}

void function(int sig, siginfo_t *siginfo, void *context){
    if(sig == SIGRTMIN){
        // przypadek kiedy rodzic otrzyma sygnal od dziecka o zgode na prace
        printf("%d: recived SIGRTMIN from process with pid: %d\n", getpid(), siginfo->si_pid);
        if(!allowed){
            children[getIndexByPID(siginfo->si_pid)].ifAllowed = true;
            requests++;
        }
        else{
            kill(siginfo->si_pid, SIGRTMAX);
        }
    }
    else if(sig == SIGRTMAX){
        printf("%d: recieved SIGRTMAX from process with pid: %d\n", getpid(), siginfo->si_pid);

    }
    else if (SIGRTMIN < sig < SIGRTMAX) { // Got SIGRT from child
        printf("%d: recieved %d from process with pid: %d\n", getpid(), sig, siginfo->si_pid);

    }
    else if (sig == SIGINT){
        if (kill(newPid, SIGINT) == -1){
            fprintf(stderr, "Problems with killing \n");
            exit(1);
        }
        exit(0);
    }

}

int getIndexByPID(int pid) {
    for (int i = 0; i < N; i++){
        if ((int) children[i].pid == pid){
            return i;
        }
    }
    return -1;
}

void allowSending(int l) {
    //jezeli dostalismy pozwolenie na wyslanie od dziecka do rodzica to puszczamy SIGRTMAX
    // oraz jezeli doczekalismy sie K requestow
    if (allowed){
        for (int i = 0; i < l; i++){
            if(children[i].ifAllowed){
                kill(children[i].pid, SIGRTMAX);
            }
        }
    }
}

void waitForRequest(int K) {
    // czekamy na K prosb
    while(1){
        if (requests == K) break;
    }
    allowed = true;
}

int randomSignal() {
    int min = SIGRTMIN;
    int max = SIGRTMAX;
    // zastrzezamy min i max sygnaly
    return rand() % (max - min - 1) + min + 1;

}

bool parseArguments(int argc, char **argv) {
    if (argc != 3) return false;
    if (!isNumber(argv[1])) return false;
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
