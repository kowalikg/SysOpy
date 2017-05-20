//
// Created by gaba on 5/10/17.
//

#ifndef ZAD1_QUEUE_H
#define ZAD1_QUEUE_H

#include <stdbool.h>

#include <sys/types.h>

void initailiseQueue(pid_t * queue, int size);

bool isEmpty(pid_t * queue);

bool isFull(pid_t * queue);

pid_t gaveSitDoorbellMan(pid_t *queue);

void pushDoorbell(pid_t *queue, pid_t client);

bool pushToQueue(pid_t * queue, pid_t client);

pid_t popFromQueue(pid_t * queue);
#endif //ZAD1_QUEUE_H
