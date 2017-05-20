#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include "semaphores_helper.h"
#include <sys/mman.h>
#include <unistd.h>
#include "queue.h"

int chairs;
sem_t * barber;
sem_t * queue;
sem_t * cut;
pid_t * segmentAdress;

bool parseArguments(int argc, char **argv);
bool isNumber(char *string);
void exitProgram();

void createSemaphores();

void initializeSharedMemory();

void cutClient(pid_t client);

int main(int argc, char **argv)  {
    if(!parseArguments(argc, argv)){
        fprintf(stderr, "Invalid arguments\n");
        exit(1);
    }
    chairs = atoi(argv[1]);

    signal(SIGINT, exitProgram);
    signal(SIGTSTP, exitProgram);

    createSemaphores();
    initializeSharedMemory();

    while(1){
        printf("Golibroda zasypia: %d\n", timestamp());
        sem_wait(barber);
        printf("Golibroda wstaje: %d\n", timestamp());

        sem_wait(queue);

        pid_t client = gaveSitDoorbellMan(segmentAdress);
        sem_post(queue);

        cutClient(client);

        do {
            sem_wait(queue);

            client = popFromQueue(segmentAdress);
            printf("Golibroda bierze nastepnego klienta %d: %d\n", client, timestamp());

            if (client != -1) {
                cutClient(client);
                sem_post(queue);
            }
        }
        while(client != -1);
        sem_post(queue);
        sem_wait(barber);
    }

    return 0;
}

void cutClient(pid_t client) {
    printf("Golibroda zaczyna golic klienta %d: %d\n", client, timestamp());
    // zdejmujemy blokade ze strzyzenia


    sem_post(cut);
    printf("Golibroda skonczyl golic klienta %d: %d\n", client, timestamp());

}

void initializeSharedMemory() {
    int segmentID = shm_open(MEMORY_NAME, O_CREAT | O_RDWR | O_EXCL, 0600);
    if (segmentID == -1) fprintf(stderr, "Cannot get segmentID\n");

    if(ftruncate(segmentID, (chairs + 4) * sizeof(pid_t)) == -1) fprintf(stderr, "Cannot allocate memory\n");

    segmentAdress = mmap(NULL, (chairs + 4) * sizeof(pid_t), PROT_READ | PROT_WRITE, MAP_SHARED, segmentID, 0);
    if (segmentAdress == (pid_t *) -1) fprintf(stderr, "Cannot create segment\n");

    initailiseQueue(segmentAdress, chairs);

}

void createSemaphores() {
    queue = sem_open(QUEUE_NAME, O_CREAT | O_RDWR | O_EXCL, 0600, 1);

    if (queue == SEM_FAILED){
        fprintf(stderr, "Cannot create semaphore Queue\n");
        exit(1);
    }
    barber = sem_open(BARBER_NAME, O_CREAT | O_RDWR | O_EXCL, 0600, 0);
    if (barber == SEM_FAILED){
        fprintf(stderr, "Cannot create semaphore Pillow\n");
        exit(1);
    }
    cut = sem_open(CUT_NAME, O_CREAT | O_RDWR | O_EXCL, 0600, 0);
    if (cut == SEM_FAILED){
        fprintf(stderr, "Cannot create semaphore Cut\n");
        exit(1);
    }

}

void exitProgram(){
    printf("Recevied stop signal\n");
    if (munmap(segmentAdress, (chairs + 4) * sizeof(int)) == -1) fprintf(stderr, "Cannot unmap segment memory\n");
    if (shm_unlink(MEMORY_NAME) == -1) fprintf(stderr, "Cannot delete memory\n");
    if (sem_close(queue) == -1) fprintf(stderr, "Cannot close queue\n");
    if (sem_unlink(QUEUE_NAME) == -1) fprintf(stderr, "Cannot delete queue\n");
    if (sem_close(barber) == -1) fprintf(stderr, "Cannot close barber\n");
    if (sem_unlink(BARBER_NAME) == -1) fprintf(stderr, "Cannot delete barber\n");
    if (sem_close(cut) == -1) fprintf(stderr, "Cannot close cut\n");
    if (sem_unlink(CUT_NAME) == -1) fprintf(stderr, "Cannot delete cut\n");

    exit(0);
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
