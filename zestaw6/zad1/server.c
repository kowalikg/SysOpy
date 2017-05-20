#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include "macros.h"

bool wait = true;

int serverQueue;

static int clientsQueues[CLIENTS];
static int clientsAmount;

void waitForMessage();

void createQueue();

void deleteQueue();

void parseMessage(struct msgbuf msgbuf);

void pushClient(struct msgbuf msgbuf);

void exitServer();

void sendMessage(struct msgbuf msgbuf);

int getIndexByQueue(int queue);

void deleteClient(struct msgbuf msgbuf);

int main() {
    printf("Server: launched!\n");
    signal(SIGINT, exitServer);
    signal(SIGTSTP, exitServer);
    createQueue();
    while(wait){
        waitForMessage();
    }
    deleteQueue();
    return 0;

}

void deleteQueue() {
    if (msgctl(serverQueue, IPC_RMID, NULL) == -1) { // usuwamy kolejke
        fprintf(stderr, "Server: cannot delete queue\n");
        exit(1);
    }
    printf("Server: queue deleted!\n");
}

void createQueue() {
    __key_t key = ftok(HOME, PROJ_ID);

    serverQueue = msgget(key, 0600 | IPC_CREAT); // tworzymy kolejke z prawami odczytu i zapisu, jezeli nie ma to ma ja stworzyc

    printf("Server: queue %d created!\n", serverQueue);
}

void waitForMessage() {
    struct msgbuf msg;

    msgrcv(serverQueue, &msg, MSGBUF_SIZE, 0, 0); // kolejka, bufor, rozmiar buffora, odbierz najdłużej oczekujący komunikat (pierwszy oczekujący w kolejce), bez flag
    parseMessage(msg);
}

void parseMessage(struct msgbuf msgbuf) {

    switch (msgbuf.mtype){
        case REQUEST:
            pushClient(msgbuf);
            break;
        case ECHO:
        case UP:
        case TIME:
            sendMessage(msgbuf);
            break;
        case EXIT:
            exitServer();
            break;
        case DELETE:
            deleteClient(msgbuf);
            break;
        default:
            printf("Server: unknown comand\n");
            break;
    }
}

void deleteClient(struct msgbuf msgbuf) {
    int index = getIndexByQueue(msgbuf.queue);

    printf("Server: client %d disconnected\n", msgbuf.queue);

    clientsQueues[index] = 0;
    clientsAmount--;

}

void sendMessage(struct msgbuf msgbuf) {
    int index = getIndexByQueue(msgbuf.queue);
    if (index == -1){
        fprintf(stderr, "Server: problem with sending message\n");
        exit(1);
    }

    if (msgbuf.mtype == UP){
        int i = 0;
        while(msgbuf.message[i] != '\0'){
            msgbuf.message[i] = toupper(msgbuf.message[i]);
            i++;
        }
    }
    else if (msgbuf.mtype == TIME){
        struct tm *t;
        time_t currentTime;
        char text[100];

        currentTime = time(NULL);
        t = localtime(&currentTime);
        strftime(text, sizeof(text) - 1, "%d %m %Y %H:%M", t);
        strcpy(msgbuf.message, text);
    }
    msgsnd(clientsQueues[index], &msgbuf, MSGBUF_SIZE, 0);

}

int getIndexByQueue(int queue) {
    int index = 0;
    while (index < CLIENTS){
        if(clientsQueues[index] == queue)
            return index;
        index++;
    }
    return -1;
}

void exitServer() {
    wait = false;
}

void pushClient(struct msgbuf msgbuf) {
    if (clientsAmount == CLIENTS){
        printf("Server: queue full\n");
        msgbuf.mtype = QUEUE_FULL;
    }
    else{
        int i;
        for (i = 0; i < CLIENTS; i++){
            if (clientsQueues[i] == 0)
                break;
        }
        clientsAmount++;
        printf("Server: client %d accpeted\n", msgbuf.queue);
        clientsQueues[i] = msgbuf.queue;
        msgbuf.mtype = REQUEST_ACCEPTED;

    }
    msgsnd(msgbuf.queue, &msgbuf, MSGBUF_SIZE, 0);
}
