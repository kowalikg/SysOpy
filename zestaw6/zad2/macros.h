//
// Created by gaba on 4/27/17.
//
#include <stdlib.h>
#ifndef ZAD2_MACROS_H
#define ZAD2_MACROS_H
//
// Created by gaba on 4/24/17.
//
#define SERVER_QUEUE_NAME "/queue_server"
#define CLIENTS 2
#define MAX_QUEUE_SIZE 10
#define MESSAGE_SIZE 256


#define REQUEST 1
#define ECHO 2
#define UP 3
#define TIME 4
#define EXIT 5
#define DELETE 6


#define QUEUE_FULL 7
#define REQUEST_ACCEPTED 8

struct msgbuf { // moja struktura wiadomosci (musi zawierac long mtype)
    long mtype;
    char message[MESSAGE_SIZE];
    char name[MESSAGE_SIZE];

};

#define MSGBUF_SIZE (sizeof(struct msgbuf)-sizeof(long))


#endif //ZAD2_MACROS_H
