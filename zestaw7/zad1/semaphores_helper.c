//
// Created by gaba on 5/10/17.
//

#include <sys/sem.h>
#include <time.h>
#include "semaphores_helper.h"

int down(int semaphoresID, unsigned short semaphoreNumber){
    struct sembuf sops;
    sops.sem_num = semaphoreNumber;
    sops.sem_op = -1;
    sops.sem_flg = 0;

    return semop(semaphoresID, &sops, 1); // ID, sturct, how many operations

}
int up(int semaphoresID, unsigned short semaphoreNumber){
    struct sembuf sops;
    sops.sem_num = semaphoreNumber;
    sops.sem_op = 1;
    sops.sem_flg = 0;

    return semop(semaphoresID, &sops, 1); // ID, sturct, how many operations
}
__time_t timestamp() {
    struct timespec timestamp;
    clock_gettime(CLOCK_MONOTONIC, &timestamp);
    return timestamp.tv_nsec/1000;
}