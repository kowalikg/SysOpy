//
// Created by gaba on 5/9/17.
//

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>
#include "semaphores_helper.h"
#include "queue.h"


int clientsCounter = 0;
pid_t * segmentAddress;
int semaphoreID;

bool parseArgumentsClient(int argc, char **argv);

key_t initKey();
bool argIsNumber(char *string);

void initaliseSharedMemory(__key_t key);

void clientsSimulator(int clients, int cuts);

void goToBarber(int cuts);

bool canEnter();

int main(int argc, char **argv) {
    if (!parseArgumentsClient(argc, argv)) {
        fprintf(stderr, "Invalid arguments\n");
        exit(1);
    }

    int clients = atoi(argv[1]);
    int cuts = atoi(argv[2]);

    __key_t key = initKey();
    initaliseSharedMemory(key);

    clientsSimulator(clients, cuts);

    while(clientsCounter < clients){
        wait(NULL);
        clientsCounter++;
    }
}

void clientsSimulator(int clients, int cuts) {
    for(int i = 0; i < clients; ++i) {
        if(fork() == 0) {
            goToBarber(cuts);
            exit(0);
        }
    }
}

void goToBarber(int cuts) {

    int cutsCounter = 0;

    while(cutsCounter < cuts){
        if(canEnter()) {
            down(semaphoreID, CUTTING_IN_PROGRESS);
            cutsCounter++;
        }
    }
    printf("Klient %d opuszcza golibrode: %d\n", getpid(), timestamp());

}

bool canEnter() {

    down(semaphoreID, WAITING_QUEUE); //blokada kolejki

    int barberStatus = semctl(semaphoreID, BARBER_PILOW, GETVAL);

    if (barberStatus == -1) return false;
    if (barberStatus == 0){
        printf("Klient %d budzi golibrode: %d\n", getpid(), timestamp());
        pushDoorbell(segmentAddress, getpid());
        up(semaphoreID, BARBER_PILOW);
        up(semaphoreID, BARBER_PILOW);
    }
    else {
        if (!pushToQueue(segmentAddress, getpid())){
            printf("Klient %d nie ma miejsca w kolejce: %d\n", getpid(), timestamp());
            up(semaphoreID, WAITING_QUEUE);
            return false;
        }
        printf("Klient %d siada do kolejki: %d\n", getpid(), timestamp());
    }
    up(semaphoreID, WAITING_QUEUE); // odblokowanie kolejki
    return true;
}

void initaliseSharedMemory(__key_t key) {
    int segmentID = shmget(key, 0, 0); // 0-> dostep do juz utworzonej pamieci
    if (segmentID == -1){
        fprintf(stderr, "Cannot get memory segment\n");
        exit(1);
    }
    segmentAddress = shmat(segmentID, NULL, 0); // segment, adres, flagi
    if (segmentAddress == (pid_t *) -1){
        fprintf(stderr, "Cannot get segment address\n");
        exit(1);
    }
    semaphoreID = semget(key, 0, 0);
    if (semaphoreID == -1){
        fprintf(stderr, "Cannot take semaphore ID\n");
        exit(1);
    }
}

key_t initKey() {
    return ftok(".", PATH_NUMBER);
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