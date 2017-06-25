#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>
#include <pthread.h>
#include "numbers.h"

#include <sys/epoll.h>
#include <unistd.h>

int portNumber;
char * filePath;

int clientsCounter = 0;
struct client clients[MAX_CLIENTS];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int epollDescriptor;
struct epoll_event event;
struct message Message;

int webSocket;
int unixSocket;

int currentOperationID = START_OPERATION_ID;

bool createThreads(pthread_t listener, pthread_t pinger);
bool isNumber(char *string);
bool parseArguments(int argc, char **argv);
void sendToRandomClient(struct operation operation);
void stop();
void printResult();

void initialiseLocal();

void addNewClient(struct sockaddr_storage sockaddr, socklen_t length);

void removeClient(int index);

int getIndexByName(char buffer[100]);
int getOperant(char operant);

void initialiseWeb();

int main(int argc, char ** argv) {
    srand((unsigned int) time(NULL));

    if (!parseArguments(argc, argv)) {
        fprintf(stderr, "Invalid arguments!\n");
        exit(1);
    }

    portNumber = atoi(argv[1]);
    filePath = argv[2];

    signal(SIGINT, stop);
    signal(SIGSTOP, stop);
    atexit(stop);

    pthread_t listener;
    pthread_t pinger;

    if (!createThreads(listener, pinger)) {
        fprintf(stderr, "Cannot create threads!\n");
        exit(1);
    }
    while (true) {
        struct operation newOperation;

        newOperation.ID = currentOperationID++;
        char operant;

        scanf("%ld %c %ld", &newOperation.x, &operant, &newOperation.y);

        newOperation.type = getOperant(operant);
        if (clientsCounter != 0) {
            sendToRandomClient(newOperation);
        } else {
            printf("No clients!\n");
        }

    }
}
int getOperant(char operant) {
    if (operant == '+')
        return ADD;
    if (operant == '-')
        return SUB;
    if (operant == '*')
        return MUL;
    if (operant == '/')
        return DIV;

}

void sendToRandomClient(struct operation operation) {
    pthread_mutex_lock(&mutex);
    int index = rand() %clientsCounter;

    struct message newMessage;
    newMessage.type = OPERATION_SERVICE;

    sprintf(newMessage.buffer, "%d %d %d %d", operation.ID, operation.x, operation.y, operation.type);
    if (sendto(clients[index].net ? webSocket : unixSocket, &newMessage, sizeof(newMessage), 0 ,
               (const struct sockaddr *) &clients[index].address, clients[index].addressLength) == -1){
        perror("sending\n");
        exit(1);
    }
    pthread_mutex_unlock(&mutex);
}

bool parseArguments(int argc, char **argv) {
    if (argc != 3) return false;
    if (!isNumber(argv[1])) return false;
    return true;
}
bool isNumber(char *string) {
    int i = 0;
    while (string[i] != '\0') {
        if (string[i] >= '0' && string[i] <= '9') i++;
        else return false;
    }
    return true;
}
void stop() {
    pthread_mutex_lock(&mutex);
    close(epollDescriptor);
    shutdown(webSocket, SHUT_RDWR);
    shutdown(unixSocket, SHUT_RDWR);
    close(webSocket);
    close(unixSocket);
    unlink(filePath);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);

    exit(0);
}
void* doListen(void *args) {
    epollDescriptor = epoll_create1(0);
    if (epollDescriptor == -1){
        fprintf(stderr, "Cannot create epoll descriptor\n");;
        exit(1);
    }
    event.events = EPOLLIN | EPOLLRDHUP;

    initialiseLocal();
    initialiseWeb();

    do {
        if (epoll_wait(epollDescriptor, &event, 1, -1) == -1){
            fprintf(stderr, "Wait\n");
            exit(1);
        }
        pthread_mutex_lock(&mutex);

        struct sockaddr_storage newAddress;
        socklen_t length = (socklen_t)sizeof(struct sockaddr_storage);

        if (recvfrom(event.data.fd, &Message, sizeof(Message), MSG_WAITALL, (struct sockaddr *) &newAddress, &length) == -1){
            fprintf(stderr, "Problem z revievem\n");
            exit(1);
        }

        if (Message.type == NEW_CLIENT){
            addNewClient(newAddress, length);
        }
        if (Message.type == CLOSE_CLIENT){
            int index = getIndexByName(Message.buffer);
            removeClient(index);
        }
        if (Message.type == DOING_PING){
            int index = getIndexByName(Message.buffer);

            clients[index].active = true;
        }
        if (Message.type == OPERATION_RESULT){
            printResult();
        }
        pthread_mutex_unlock(&mutex);
    }
    while(1);
}

void initialiseWeb(){
    webSocket = socket(AF_INET,SOCK_DGRAM,0);
    if(webSocket == -1){
        fprintf(stderr, "Cannot open socket\n");
        exit(1);
    }
    struct sockaddr_in web_addr;
    web_addr.sin_family = AF_INET;
    web_addr.sin_addr.s_addr = INADDR_ANY;
    web_addr.sin_port = htons(portNumber);

    if(bind(webSocket,(struct sockaddr *)&web_addr,sizeof(web_addr)) == -1){
        fprintf(stderr, "Cannot bind\n");
        exit(1);
    }

    event.data.fd = webSocket;
    if(epoll_ctl(epollDescriptor, EPOLL_CTL_ADD, webSocket, &event)==-1){
        fprintf(stderr, "Epoll error\n");
        exit(1);
    }

}

void printResult() {
    int id = atoi(strtok(Message.buffer, " "));
    int result = atoi(strtok(NULL, " "));
    printf("Operation %d: result %d\n", id, result );
}

int getIndexByName(char buffer[100]) {
    for (int i = 0; i < clientsCounter; i++){
        if (strcmp(buffer, clients[i].name) == 0)
            return i;
    }
    return -1;
}

void removeClient(int index) {
    for (int i = index + 1; i < MAX_CLIENTS; i++){
        int j = i - 1;
        clients[j].active = clients[i].active;
        clients[j].address = clients[i].address;
        clients[j].addressLength = clients[j].addressLength;
        strcpy(clients[j].name, clients[i].name);
    }
    clientsCounter--;
}


void addNewClient(struct sockaddr_storage sockaddr, socklen_t length) {
    if (clientsCounter == MAX_CLIENTS){
        Message.type = NO_SPACE;
        if (sendto(Message.net ? webSocket : unixSocket, &Message, sizeof(Message), 0 , (const struct sockaddr *) &sockaddr, length) == -1){
            perror("sending\n");
            exit(1);
        }
    }
    else {
        for (int i = 0; i < MAX_CLIENTS; i++){
            if (strcmp(clients[i].name, Message.buffer) == 0){
                Message.type = IS_CLIENT_WITH_THAT_NAME;
                if (sendto(Message.net ? webSocket : unixSocket, &Message, sizeof(Message), 0 , (const struct sockaddr *) &sockaddr, length) == -1) {
                    perror("sending\n");
                    exit(1);
                }
                return;
            }
        }

        clients[clientsCounter].active = true;
        clients[clientsCounter].net = Message.net ? true : false;
        clients[clientsCounter].address = sockaddr;
        clients[clientsCounter].addressLength = length;
        strcpy(clients[clientsCounter++].name, Message.buffer);

        Message.type = ADD_CLIENT_SUCCESS;
        if (sendto(Message.net ? webSocket : unixSocket, &Message, sizeof(Message), 0 , (const struct sockaddr *) &sockaddr, length) == -1) {
            perror("sending\n");
            exit(1);
        }

    }

}

void initialiseLocal() {
    unixSocket = socket(AF_UNIX,SOCK_DGRAM,0);
    if(unixSocket == -1){
        perror("Socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_un local_addr;
    local_addr.sun_family = AF_UNIX;
    strcpy(local_addr.sun_path, filePath);
    unlink(local_addr.sun_path);
    if(bind(unixSocket,(struct sockaddr *)&local_addr,sizeof(local_addr)) == -1){
        exit(EXIT_FAILURE);
    }
    event.data.fd = unixSocket;
    if(epoll_ctl(epollDescriptor, EPOLL_CTL_ADD, unixSocket, &event) == -1){
        exit(EXIT_FAILURE);
    }
}

void* doPing(void *args) {
    struct message newMessage;
    newMessage.type = DOING_PING;

    while (1) {
        pthread_mutex_lock(&mutex);

        for (int i = 0; i < clientsCounter; i++) {
            clients[i].active = false;
            if (sendto(clients[i].net ? webSocket : unixSocket, &newMessage, sizeof(newMessage), 0, &clients[i].address, clients[i].addressLength) == -1) {
                perror("sending\n");
                exit(1);
            }
        }
        pthread_mutex_unlock(&mutex);
        sleep(PING_DELAY);
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < clientsCounter; i++) {
            if (!clients[i].active) {
                int index = getIndexByName(clients[i].name);
                removeClient(index);
            }
        }
        pthread_mutex_unlock(&mutex);
    }
}
bool createThreads(pthread_t listener, pthread_t pinger) {
    if (pthread_create(&listener, NULL, doListen, NULL) != 0){
        fprintf(stderr, "Cannot create listener\n");
        return false;
    }
    if (pthread_create(&pinger, NULL, doPing, NULL) != 0){
        fprintf(stderr, "Cannot create pinger message\n");
        return false;
    }
    return true;
}
