//
// Created by gaba on 5/10/17.
//

#ifndef ZAD1_SEMAPHORES_HELPER_H
#define ZAD1_SEMAPHORES_HELPER_H

#include <sys/sem.h>

#define WAITING_QUEUE 0
#define CUTTING_IN_PROGRESS 1
#define BARBER_PILOW 2

#define PATH_NUMBER rand() % 100

int down(int semaphoresID, unsigned short semaphoreNumber);
int up(int semaphoresID, unsigned short semaphoreNumber);
__time_t timestamp();

#endif //ZAD1_SEMAPHORES_HELPER_H
