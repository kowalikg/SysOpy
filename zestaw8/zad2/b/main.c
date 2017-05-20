#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

pthread_t * threadArray;
int delay = 1;
void signalHandler(int signal);
void* fun1();
void* fun2();

int main() {
    signal(SIGFPE, signalHandler);

    threadArray = malloc(10 * (sizeof (pthread_t)));

    for (int i = 0; i < 5; i++){
        int status = 0;
        if (i != 0){
            status = pthread_create(&threadArray[i], NULL, fun1, NULL);
        }
        else{
            status = pthread_create(&threadArray[i], NULL, fun2, NULL);
        }
        if(status != 0){
            fprintf(stderr, "Cannot create thread %d\n", i);
            exit(1);
        }
    }
    delay = 0;
    for (int i = 0; i < 10; i++){
        pthread_join(threadArray[i], NULL);
    }
    return 0;
}
void *fun1() {
    while(delay);

    for (int i = 5; i >= 0; i--){
        sleep(1);
        printf("To end: %d\n", i);
    }
    return (void *) 0;
}

void *fun2() {
    while(delay);
    int zero = 0;
    int result = 10 / zero;
    return (void *) 0;

}
void signalHandler(int signal){
    printf("Thread: %lu with pid %d has recived %d\n",
    pthread_self(), getpid(), signal);
}