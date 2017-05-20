#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include "queue.h"
#include "semaphores_helper.h"

pid_t * segmentAddress;
int semaphoresID, segmentID;

void cutClient(int semaphoresID, pid_t i);
bool parseArguments(int argc, char **argv);
bool isNumber(char *string);
void exitProgram();
int createSemaphores(int chairs);
void initialiseSharedMemory(key_t key, int chairs);
void companySimulator(int i);

int main(int argc, char** argv) {

    if(!parseArguments(argc, argv)){
        fprintf(stderr, "Invalid arguments\n");
        exit(1);
    }

    int chairs = atoi(argv[1]);

    signal(SIGINT, exitProgram);
    signal(SIGTSTP, exitProgram);

    semaphoresID = createSemaphores(chairs);

    companySimulator(semaphoresID);

    return 0;
}

int createSemaphores(int chairs) {
    key_t key = ftok(".", PATH_NUMBER); // PATHNAME + JAKAS LICZBA

    int semaphoresID = semget(key, 4, IPC_CREAT | 0600); // 4 semafory, jak nie ma to utworz do zapisu i odczytu

    if (semaphoresID == -1){
        fprintf(stderr, "Cannot create semaphores set\n");
        exit(1);
    }

    if(semctl(semaphoresID, BARBER_PILOW, SETVAL, 0) == -1 || semctl(semaphoresID, CUTTING_IN_PROGRESS, SETVAL, 0) || //barber spi i nie robi strzyzenia
       semctl(semaphoresID, WAITING_QUEUE, SETVAL, 1) == -1){
        sprintf(stderr, "Cannot initialise semaphores values\n");
        exit(1);
    }

    initialiseSharedMemory(key, chairs);

    return semaphoresID;

}

void companySimulator(int semaphoresID) {
    while(1){
        printf("Golibroda zasypia: %d\n", timestamp());
        down(semaphoresID, BARBER_PILOW);
        // this goes after another process do up to BARBER_PILOW
	printf("Golibroda wstaje: %d\n", timestamp());
        down(semaphoresID, WAITING_QUEUE);
        // before doing anything with queue we have to block it

        pid_t client = gaveSitDoorbellMan(segmentAddress);
        // the client woke up barber and sat on chair

        up(semaphoresID, WAITING_QUEUE);
        // unblock queue
        cutClient(semaphoresID, client);

        do {
            down(semaphoresID, WAITING_QUEUE);

            client = popFromQueue(segmentAddress);
            printf("Golibroda bierze nastepnego klienta %d: %d\n", client, timestamp());
            up(semaphoresID, WAITING_QUEUE);

            if (client != -1)
                cutClient(semaphoresID, client);

            up(semaphoresID, WAITING_QUEUE); // czekamy na klienta
        }
        while(client != -1);
        down(semaphoresID, BARBER_PILOW);

    }

}

void cutClient(int semaphoresID, pid_t i) {
    printf("Golibroda zaczyna golic klienta %d: %d\n", i, timestamp());
    // zdejmujemy blokade ze strzyzenia
    up(semaphoresID, CUTTING_IN_PROGRESS);
    printf("Golibroda skonczyl golic klienta %d: %d\n", i, timestamp());

}

void initialiseSharedMemory(key_t key, int chairs) {
    segmentID = shmget(key, (chairs + 4) * sizeof(pid_t), IPC_CREAT | 0600);
    if (segmentID == -1){
        fprintf(stderr, "Cannot get memory segment\n");
        exit(1);
    }

    segmentAddress = shmat(segmentID, NULL, 0); // segment, adres, flagi
    if (segmentAddress == (pid_t *) -1){
        fprintf(stderr, "Cannot get segment address\n");
        exit(1);
    }
    initailiseQueue(segmentAddress, chairs);

    //    tests
     //  pushToQueue(segmentAddress, 9354);
    //    pushToQueue(segmentAddress, 93254);
    //    printf("%d\n", popFromQueue(segmentAddress));
    //    printf("%d\n", popFromQueue(segmentAddress));

}

bool parseArguments(int argc, char **argv) {
    if (argc != 2) return false;
    if (!isNumber(argv[1])) return false;
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

void exitProgram(){
    printf("Recevied stop signal\n");

    shmdt(segmentAddress);
    shmctl(segmentID,IPC_RMID,NULL);
    semctl(semaphoresID,0,IPC_RMID);
    exit(0);
}
