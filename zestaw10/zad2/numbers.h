//
// Created by gaba on 5/30/17.
//

#ifndef ZAD1_NUMBERS_H
#define ZAD1_NUMBERS_H

#define MAX_CLIENTS 3
#define MAX_BUFFER 100
#define START_OPERATION_ID 1000


#include <signal.h>
#include <pthread.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>


struct client{
    bool active;
    char name[MAX_BUFFER];
    struct sockaddr_storage address;
    bool net;
    socklen_t addressLength;
};
struct operation{
    int ID;
    int type;
    int x;
    int y;
};
struct message{
    int type;
    bool net;
    char buffer[MAX_BUFFER];
};

#define PING_DELAY 5

#define ADD 1
#define SUB 2
#define MUL 3
#define DIV 4

#define OPERATION_COUNTER 5

#define OPERATION_SERVICE 1
#define DOING_PING 2
#define NEW_CLIENT 3
#define NO_SPACE 4
#define ADD_CLIENT_SUCCESS 5
#define IS_CLIENT_WITH_THAT_NAME 6
#define CLOSE_CLIENT 7
#define OPERATION_RESULT 8


#endif //ZAD1_NUMBERS_H
