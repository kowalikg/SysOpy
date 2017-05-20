#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

int counter = 1;
void changeCounter(int sign_no) {
    char * message = "\n";
    write(STDOUT_FILENO, message, strlen(message));
    counter*=-1;

}
void stop(){
    char * message = "\nPrzechwycono sygnal SIGINT -> koniec programu\n";
    write(STDOUT_FILENO, message, strlen(message));
    exit(0);
}

int main() {

    struct sigaction act;
    act.sa_handler = changeCounter;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    signal(SIGINT, stop);
    sigaction(SIGTSTP, &act, NULL);

    char toDisplay = 'a';
    while(1){
        printf("%c\n", toDisplay);

        sleep(1);
        if (counter == -1 && toDisplay =='a'){
            toDisplay = 'z';
            continue;
        }
        if (counter == 1 && toDisplay =='z'){
            toDisplay = 'a';
            continue;
        }
        toDisplay+=counter;

    }

    return 0;
}