#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wait.h>
#include "semaphores_helper.h"
#include "queue.h"

//
// Created by gaba on 5/11/17.
//
sem_t * barber;
sem_t * queue;
sem_t * cut;

pid_t * segmentAddress;

void getSemaphores();
bool parseArgumentsClient(int argc, char **argv);
bool argIsNumber(char *string);

void getMemory();

void clientsSimulator(int clients, int cuts);

void goToBarber(int cuts);

bool canEnter();

void freeMemory(int clients);

int main(int argc, char **argv) {
    if (!parseArgumentsClient(argc, argv)) {
        fprintf(stderr, "Invalid arguments\n");
        exit(1);
    }

    int clients = atoi(argv[1]);
    int cuts = atoi(argv[2]);

    getSemaphores();
    getMemory();

    clientsSimulator(clients, cuts);

    int clientsCounter = 0;
    while(clientsCounter < clients){
        wait(NULL);
        clientsCounter++;
    }
    exit(0);

}
void clientsSimulator(int clients, int cuts) {
    for(int i = 0; i < clients; ++i) {
        if(fork() == 0) {
            goToBarber(cuts);
            freeMemory(clients);
            exit(0);
        }
    }
}

void freeMemory(int clients) {
    if (sem_close(queue) == -1) fprintf(stderr, "Cannot close queue\n");
    if (sem_close(barber) == -1) fprintf(stderr, "Cannot close barber\n");
    if (sem_close(cut) == -1) fprintf(stderr, "Cannot close cut\n");

    if (munmap(segmentAddress, segmentAddress[1] * sizeof(pid_t)) == -1) fprintf(stderr, "Cannot unmap segment memory\n");
}

void goToBarber(int cuts) {
    int cutsCounter = 0;
    while(cutsCounter < cuts){
        if(canEnter()) {
            sem_wait(cut);
            cutsCounter++;
        }
    }
    printf("Klient %d opuszcza golibrode: %d\n", getpid(), timestamp());

}

bool canEnter() {
    sem_wait(queue);

    int barberStatus;
    sem_getvalue(barber, &barberStatus);

    if (barberStatus == -1){
        fprintf(stderr, "Problem with barber semaphors\n");
        exit(1);
    } else if (barberStatus == 0){
        printf("Klient %d budzi golibrode: %d\n", getpid(), timestamp());
        pushDoorbell(segmentAddress, getpid());
        sem_post(barber);
        sem_post(barber);
    }
    else{
        if(!pushToQueue(segmentAddress, getpid())){
            printf("Klient %d nie ma miejsca w kolejce: %d\n", getpid(), timestamp());
            sem_post(queue);
            return false;
        } else{
            printf("Klient %d siada do kolejki: %d\n", getpid(), timestamp());
        }
    }

    sem_post(queue);

    return true;
}

void getMemory() {
    int segmentID = shm_open(MEMORY_NAME, O_RDWR, 0600);
    if (segmentID == -1) fprintf(stderr, "Cannot get segmentID\n");

    segmentAddress = mmap(NULL, sizeof(pid_t), PROT_READ | PROT_WRITE, MAP_SHARED, segmentID, 0);
    if (segmentAddress == (pid_t *) -1) fprintf(stderr, "Cannot create segment\n");

}

bool parseArgumentsClient(int argc, char **argv) {
    if (argc != 3) return false;
    if (!argIsNumber(argv[1])) return false;
    if (!argIsNumber(argv[2])) return false;

    return true;
}
bool argIsNumber(char *string) {

    int i = 0;
    while(string[i] != '\0'){
        if (string[i] >= '0' && string[i] <= '9') i++;
        else return false;
    }
    return true;
}
void getSemaphores() {
    queue = sem_open(QUEUE_NAME, O_RDWR, 0600, 0);

    if (queue == SEM_FAILED){
        fprintf(stderr, "Cannot create semaphore Queue\n");
        exit(1);
    }
    barber = sem_open(BARBER_NAME,  O_RDWR, 0600, 0);
    if (barber == SEM_FAILED){
        fprintf(stderr, "Cannot create semaphore Pillow\n");
        exit(1);
    }
    cut = sem_open(CUT_NAME, O_RDWR, 0600, 0);
    if (cut == SEM_FAILED){
        fprintf(stderr, "Cannot create semaphore Cut\n");
        exit(1);
    }

}
