//
// Created by gaba on 4/24/17.
//
#include <stdlib.h>
#ifndef ZAD1_MACROS_H
#define ZAD1_MACROS_H

#define HOME getenv("HOME") // zawartosc zmiennej srodowiskowej HOME
#define PROJ_ID rand() % 256
#define CLIENTS 10
#define MESSAGE_SIZE 256

#define QUEUE_FULL 1
#define REQUEST_ACCEPTED 2

#define REQUEST 1
#define ECHO 2
#define UP 3
#define TIME 4
#define EXIT 5
#define DELETE 6

struct msgbuf { // moja struktura wiadomosci (musi zawierac long mtype)
    long mtype;
    int queue;
    char message[MESSAGE_SIZE];

};

#define MSGBUF_SIZE (sizeof(struct msgbuf)-sizeof(long))

#endif //ZAD1_MACROS_H
