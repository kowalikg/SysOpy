//
// Created by gaba on 5/11/17.
//
#include "semaphores_helper.h"

__time_t timestamp() {
    struct timespec timestamp;
    clock_gettime(CLOCK_MONOTONIC, &timestamp);
    return timestamp.tv_nsec/1000;
}