#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include "numbers.h"
#include "sys/epoll.h"


bool parseArguments(int argc, char **argv);
bool isNumber(char *string);
void* doListen(void *args);
void* doPing(void *args);
void stop();
bool createThreads(pthread_t listener, pthread_t pinger);

void sendToRandomClient(struct operation operation);

void initialiseLocal();

void addNewClient(int descriptor, char string[100]);

void closeUnactiveClient(int fd);

int getIndexByDescriptor(int fd);

int getOperant(char operant);

void setClientAsActive(int fd);

void printResult();

void initialiseWeb();

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

int main(int argc, char ** argv) {
    srand((unsigned int) time(NULL));

    if (!parseArguments(argc, argv)){
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

    if(!createThreads(listener, pinger)){
        fprintf(stderr, "Cannot create threads!\n");
        exit(1);
    }

    while(true){
        struct operation newOperation;

        newOperation.ID = currentOperationID++;
        char operant;

        scanf("%ld %c %ld", &newOperation.x, &operant, &newOperation.y);

        newOperation.type = getOperant(operant);
        if(clientsCounter != 0){
            sendToRandomClient(newOperation);
        } else{
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
    int index;
    do {
        index = rand() %clientsCounter;
    }
    while(clients[index].descriptor == 0);

    struct message newMessage;
    newMessage.type = OPERATION_SERVICE;

    sprintf(newMessage.buffer, "%d %d %d %d", operation.ID, operation.x, operation.y, operation.type);
    send(clients[index].descriptor, &newMessage, sizeof(newMessage), 0);
    pthread_mutex_unlock(&mutex);
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

void stop(){
    pthread_mutex_lock(&mutex);
    while(clientsCounter != 0){
        closeUnactiveClient(clients[0].descriptor);
    }

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
void* doListen(void *args){
    epollDescriptor = epoll_create1(0);

    if (epollDescriptor == -1){
        fprintf(stderr, "Cannot create epoll descriptor\n");
        exit(1);
    }
    event.events = EPOLLIN | EPOLLRDHUP; // do odczytu, monitorowanie odlaczenia gniazda

    initialiseLocal();
    initialiseWeb();

    do {
        if (epoll_wait(epollDescriptor, &event, 1, -1) == -1){ // czekamy na jedno wydarzenie w nieskonczonosc
            fprintf(stderr, "Cannot wait for event\n");
            exit(1);
        }
        pthread_mutex_lock(&mutex);
        if ((event.events & EPOLLERR) != 0 || (event.events & EPOLLRDHUP) != 0){
            closeUnactiveClient(event.data.fd);
        }
        else if(event.data.fd == unixSocket || event.data.fd == webSocket){
            struct sockaddr tmpAddress;
            socklen_t length = sizeof(tmpAddress);
            int clientDescriptor = accept(event.data.fd, &tmpAddress, &length);

            if (clientDescriptor == -1){
                fprintf(stderr, "Cannot get client descriptor\n");
                exit(1);
            }

            if (recv(clientDescriptor, &Message, sizeof(Message), MSG_WAITALL) == -1){ // czekamy na wszystkie bajty
                fprintf(stderr, "Cannot recieve message\n");
                exit(1);
            }
            if (Message.type == NEW_CLIENT){
                addNewClient(clientDescriptor, Message.buffer);
            }
            else{
                close(clientDescriptor);
            }
        }
        else{ // dostalismy wiadomosc ale nie od siebie tylko od monitorowanego innego gniazda
            if (recv(event.data.fd, &Message, sizeof(Message), 0) == -1){
                fprintf(stderr, "Cannot recieve message\n");
                exit(1);
            }
            if(Message.type == CLOSE_CLIENT){
                closeUnactiveClient(event.data.fd);

            }
            else if (Message.type == OPERATION_RESULT){
                printResult();
            }
            else if (Message.type == DOING_PING){
                setClientAsActive(event.data.fd);
            }
        }
        pthread_mutex_unlock(&mutex);
    }
    while(1);

}

void initialiseWeb() {
    webSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (webSocket == -1){
        fprintf(stderr, "Cannot open socket\n");
        exit(1);
    }

    struct sockaddr_in webAddress;

    webAddress.sin_port = htons(portNumber); // przerabiamy na network byte order
    webAddress.sin_family = AF_INET;
    webAddress.sin_addr.s_addr = INADDR_ANY; // jakikolwiek posiadany przez maszyne

    if (bind(webSocket, (struct sockaddr *) & webAddress, sizeof(webAddress)) == -1){
        fprintf(stderr, "Problem with binding\n");
        exit(1);
    }

    if (listen(webSocket, MAX_CLIENTS) == -1){
        fprintf(stderr, "Problem with listening\n");
        exit(1);
    }
    event.data.fd = webSocket;

    if(epoll_ctl(epollDescriptor, EPOLL_CTL_ADD, webSocket, &event) == -1){
        fprintf(stderr, "Cannot add descriptor to epoll\n");
        exit(1);
    }
}

void printResult() {
    int id = atoi(strtok(Message.buffer, " "));
    int result = atoi(strtok(NULL, " "));
    printf("Operation %d: result %d\n", id, result );
}

void setClientAsActive(int fd) {
    int index = getIndexByDescriptor(event.data.fd);
    printf("Recived ping from %s\n", clients[index].name);

    clients[index].active = true;
}

void closeUnactiveClient(int fd) {

    int index = getIndexByDescriptor(fd);

    for (int i = index + 1; i < MAX_CLIENTS; i++){
        int j = i - 1;
        clients[j].active = clients[i].active;
        clients[j].descriptor = clients[i].descriptor;
        strcpy(clients[j].name, clients[i].name);
    }
    clientsCounter--;

    struct epoll_event event;

    event.events = EPOLLIN | EPOLLET;
    event.data.fd = fd;

    if (epoll_ctl(epollDescriptor, EPOLL_CTL_DEL, fd, &event) == -1) {
        fprintf(stderr, "Cannot delete client from epoll\n");
        exit(EXIT_FAILURE);
    }
    close(fd);


}

int getIndexByDescriptor(int fd) {
    for (int i = 0; i < MAX_CLIENTS; i++){
        if (clients[i].descriptor == fd){
            return i;
        }

    }
    return -1;
}

void addNewClient(int descriptor, char string[100]) {
    if (clientsCounter == MAX_CLIENTS){
        Message.type = NO_SPACE;
        send(descriptor, &Message, sizeof(Message), 0);
        close(descriptor);
    }
    else {
        for (int i = 0; i < MAX_CLIENTS; i++){
            if (strcmp(clients[i].name, string) == 0){
                Message.type = IS_CLIENT_WITH_THAT_NAME;
                send(descriptor, &Message, sizeof(Message), 0);
                close(descriptor);
                return;
            }
        }

        clients[clientsCounter].descriptor = descriptor;
        clients[clientsCounter].active = true;
        strcpy(clients[clientsCounter++].name, string);

        struct epoll_event newEvent;

        newEvent.events = EPOLLIN | EPOLLET;
        newEvent.data.fd = descriptor;

        // rejestrujemy wydarzenie

        if (epoll_ctl(epollDescriptor, EPOLL_CTL_ADD, descriptor, &newEvent) == -1) {
            fprintf(stderr, "Cannot add by epoll new descriptor\n");
            exit(1);
        }

        Message.type = ADD_CLIENT_SUCCESS;
        send(descriptor, &Message, sizeof(Message), 0);
    }

}

void initialiseLocal() {
    unixSocket = socket(AF_UNIX,SOCK_STREAM,0); // typ unixowy

    if(unixSocket == -1){
        fprintf(stderr, "Cannot open unix socket\n");
        exit(1);
    }

    struct sockaddr_un addressLocal;
    addressLocal.sun_family = AF_UNIX;
    strcpy(addressLocal.sun_path, filePath);
    unlink(addressLocal.sun_path);

    if(bind(unixSocket, (struct sockaddr *) & addressLocal,sizeof(addressLocal)) == -1) {
        fprintf(stderr, "Problems with binding\n");
        exit(1);
    }
    if(listen(unixSocket, MAX_CLIENTS) == -1){ // nasluchywanie max klientow
        fprintf(stderr, "Problems with listening\n");
        exit(1);
    }
    event.data.fd = unixSocket; // dodajemy do epolla obserowany socket

    if(epoll_ctl(epollDescriptor, EPOLL_CTL_ADD, unixSocket, &event)==-1){
        fprintf(stderr, "Cannot set epoll\n");
        exit(1);
    }

}

void* doPing(void *args){
    struct message newMessage;
    newMessage.type = DOING_PING;

    while(1){
        pthread_mutex_lock(&mutex);

        for (int i = 0; i < clientsCounter; i++){
            if(clients[i].descriptor != 0){
                clients[i].active = false;
                send(clients[i].descriptor, &newMessage, sizeof(newMessage), 0);
            }
        }
        pthread_mutex_unlock(&mutex);
        sleep(PING_DELAY);
        printf("Ping: \n");
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < clientsCounter; i++){
            if(!clients[i].active){
                closeUnactiveClient(clients[i].descriptor);
            }
        }
        pthread_mutex_unlock(&mutex);
    }


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