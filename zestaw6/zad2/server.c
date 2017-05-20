#include <signal.h>
#include <stdio.h>
#include <stdbool.h>
#include "macros.h"
#include <fcntl.h>
#include <mqueue.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

static int clientsQueues[CLIENTS];
static char* clientsQueuesNames[CLIENTS];
static int clientsAmount;

bool wait = true;
mqd_t serverQueue;
void createQueue();

void waitForMessage();

void deleteQueue();
void exitServer();

void parseMessage(struct msgbuf msgbuf);

void pushClient(struct msgbuf msgbuf);

void deleteClient(struct msgbuf msgbuf);

void sendMessage(struct msgbuf msgbuf);

int main() {
    printf("Server: launched!\n");
    signal(SIGINT, exitServer);
    signal(SIGTSTP, exitServer);
    createQueue();
    while(wait){
        waitForMessage();
    }
    return 0;
}

void exitServer(){
    printf("Server: shoutdown\n");
    wait = false;
    for (int i = 0; i < CLIENTS; i++) {
        if (clientsQueues[i])
            mq_close(clientsQueues[i]);
    }
    deleteQueue();

}
void deleteQueue() {
    mq_close(serverQueue);
    mq_unlink(SERVER_QUEUE_NAME);
}

void waitForMessage() {
    struct msgbuf msg;

    mq_receive(serverQueue, (char*) &msg, MSGBUF_SIZE, NULL);
    // Dane zapisuje do ptr, a priorytet do priop (o ile priop nie jest NULL).
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
            break;
    }

}
void pushClient(struct msgbuf msgbuf) {
    int clientQueue;
    if (clientsAmount == CLIENTS){
        printf("Server: queue full\n");
        clientQueue = mq_open(msgbuf.name, O_WRONLY);
        msgbuf.mtype = QUEUE_FULL;
    }
    else{
        int i;
        for (i = 0; i < CLIENTS; i++){
            if (clientsQueues[i] == 0)
                break;
        }
        clientsAmount++;
        printf("Server: client %s accpeted\n", msgbuf.name);
        clientsQueues[i] = mq_open(msgbuf.name, O_WRONLY);
        clientsQueuesNames[i] = msgbuf.name;
        clientQueue = clientsQueues[i];
        msgbuf.mtype = REQUEST_ACCEPTED;

    }
    mq_send(clientQueue, (char*) &msgbuf, MSGBUF_SIZE, 0);
}
int getIndexByQueue(char * name) {

    int index = 0;
    while (index < CLIENTS){
        if(strcmp(clientsQueuesNames[index], name) == 0){
            return index;
        }

        index++;
    }
    return -1;
}

void sendMessage(struct msgbuf msgbuf) {
    int index = getIndexByQueue(msgbuf.name);
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
    mq_send(clientsQueues[index], (char*) &msgbuf, MSGBUF_SIZE, 0);

}

void deleteClient(struct msgbuf msgbuf) {
    int index = getIndexByQueue(msgbuf.name);
    mq_close(clientsQueues[index]);
    printf("Server: client %s disconnected\n", msgbuf.name);

    clientsQueues[index] = 0;
    clientsAmount--;
}

void createQueue() {
    struct mq_attr queueInfo;
    queueInfo.mq_msgsize = MSGBUF_SIZE;
    queueInfo.mq_maxmsg = MAX_QUEUE_SIZE;

    serverQueue = mq_open(SERVER_QUEUE_NAME, O_CREAT | O_RDONLY, 0600, &queueInfo);

    if (serverQueue == -1){
        fprintf(stderr, "Server: cannot create queue\n");
        exit(1);
    }
    printf("Server: created queue %s\n", SERVER_QUEUE_NAME);
}
