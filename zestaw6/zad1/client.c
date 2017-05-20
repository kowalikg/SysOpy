//
// Created by gaba on 4/24/17.
//
#include <stdlib.h>
#include <signal.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "stdio.h"
#include "macros.h"

int serverQueue;
int clientQueue;

int stringToNumber(char *string);

void createClientQueue();

void deleteClientQueue();

bool connectToServer();

void parseChoise(int choise);

void sendMessageToServer(int mode);

void getMessage();

int main() {

    signal(SIGINT, deleteClientQueue);
    signal(SIGTSTP, deleteClientQueue);

    createServerQueue();
    createClientQueue();

    if (!connectToServer()) exit(1);
    int choise;
    char buffer[1];
    do {
        printf("Welcome on Gabrysia's super program: client\n");
        printf("Select option:\n");
        printf("To close: 1\n");
        printf("To send ECHO: 2\n");
        printf("To send UP: 3\n");
        printf("To get TIME: 4\n");
        printf("To kill server: 5\n");
        scanf("%s", buffer);

        choise = stringToNumber(buffer);
        if (choise != 1) parseChoise(choise);
    }
    while (choise != 1 && choise != 5);

    deleteClientQueue();
}

void getMessage() {
    struct msgbuf msgbuf;
    msgrcv(clientQueue, &msgbuf, MSGBUF_SIZE, 0 , 0);
    printf("Client: server sends: %s\n", msgbuf.message);
    sleep(1);
}

void parseChoise(int choise) {
    switch (choise){
        case ECHO:
            sendMessageToServer(ECHO);
            getMessage();
            break;
        case UP:
            sendMessageToServer(UP);
            getMessage();
            break;
        case TIME:
            sendMessageToServer(TIME);
            getMessage();
            break;
        case EXIT:
            sendMessageToServer(EXIT);
            break;
        default:
            printf("Client: unknown comand\n");
            break;
    }

}

void deleteClientQueue() {

    sendMessageToServer(DELETE);

    if (msgctl(clientQueue, IPC_RMID, NULL) == -1) { // usuwamy kolejke
        fprintf(stderr, "Client: cannot delete queue\n");
        exit(1);
    }
    printf("Client: queue deleted!\n");

    exit(0);
}
void sendMessageToServer(int mode) {
    struct msgbuf msgbuf;

    if (mode == ECHO || mode == UP){
        printf("Write message: max 256 chars\n");
        scanf("%s", &msgbuf.message);
    }

    msgbuf.queue = clientQueue;
    msgbuf.mtype = mode;

    if (msgsnd(serverQueue, &msgbuf, MSGBUF_SIZE, 0) == -1){
        fprintf(stderr, "Client: problem with sending message to server\n");
        exit(1);
    }

}
bool connectToServer() {
    struct msgbuf msgbuf;

    msgbuf.queue = clientQueue;
    msgbuf.mtype = REQUEST;

    if(msgsnd(serverQueue, &msgbuf, MSGBUF_SIZE, 0) == -1) {
        fprintf(stderr, "Client: problem with connecting to server\n");
        return false;
    }
    printf("Client: connecting to server...\n");
    msgrcv(clientQueue, &msgbuf, MSGBUF_SIZE, 0 , 0);
    if (msgbuf.mtype == QUEUE_FULL){
        fprintf(stderr, "Client: server's queue full\n");
        return false;
    }

    printf("Client %d: connected to server...\n", msgbuf.queue);

    return true;


}

void createClientQueue() {
    /*
     *  Jeśli użyliśmy IPC_PRIVATE, to wartość zwróconą przez msgget zwykle w jakiś sposób
        przekazujemy innemu procesowi (np. zapisując ją do innej, z góry ustalonej kolejki, która
        została utworzona w oparciu o ftok i ścieżkę na dysku)
     */
    clientQueue = msgget(IPC_PRIVATE, 0600);
    if (clientQueue == -1){
        fprintf(stderr, "Client: cannot create queue\n");
        exit(1);
    }
}

void createServerQueue() {
    __key_t key = ftok(HOME, PROJ_ID);
    serverQueue = msgget(key, 0); // tworzymy kolejke

    if (serverQueue == -1){
        fprintf(stderr, "Client: cannot connect to server's queue\n");
        exit(1);
    }

}
int stringToNumber(char *string) {
    int number = 0;
    int i = 0;

    while(string[i] != '\0'){
        number = 10 * number + (string[i] - '0');
        i++;
    }
    return number;
}