//
// Created by gaba on 5/10/17.
//

#include <printf.h>
#include "queue.h"

void initailiseQueue(pid_t * queue, int size){
    queue[1] = size + 3; // size of queue
    queue[2] = 0; // counter for current clients (max size)
}

bool isEmpty(pid_t * queue){
    if (queue[2])
        return false;
    return true;
}

bool isFull(pid_t * queue){
    if (queue[2] == queue[1] - 3)
        return true;
    return false;
}

pid_t gaveSitDoorbellMan(pid_t *queue){
    return queue[0];
}

void pushDoorbell(pid_t *queue, pid_t client){
    queue[0] = client;
}

bool pushToQueue(pid_t * queue, pid_t client){
    if (isFull(queue))
        return false;

    // rozmiar++ + przesuniecie
    queue[queue[2]++ + 3] = client;

    return true;
}

pid_t popFromQueue(pid_t * queue){
    if (isEmpty(queue))
        return -1;

    pid_t value = queue[3];

    for (int i = 3; i < queue[2] + 3; i++){
        queue[i] = queue[i+1];
    }

    queue[2]--;

    return value;
}
