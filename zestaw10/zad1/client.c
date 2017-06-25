#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "numbers.h"

bool parseArguments(int argc, char **argv);
bool isNumber(char *string);
void sendHello();
void countAndSend();
void answerPing();
void launchWeb();
void launchUnix();

int mode;
char * name;
char * address;
int port;
int clientSocket = -1;
struct message Message;
void exitClient();

int main(int argc, char ** argv) {

    if (!parseArguments(argc, argv)){
        fprintf(stderr, "Invalid arguments\n");
        exit(1);
    }
    name = argv[1];
    mode = atoi(argv[2]);
    address = argv[3];

    if (mode){
        port = atoi(argv[4]);
        launchWeb();
    }
    else {
        launchUnix();
    }

    signal(SIGINT, exitClient);
    signal(SIGSTOP, exitClient);
    sendHello();

    do {
        if (recv(clientSocket, &Message, sizeof(Message), 0) == -1){
            fprintf(stderr, "Cannot recieve message\n");
            exit(1);
        }
        if (Message.type == OPERATION_SERVICE){
            countAndSend();
        }
        else if (Message.type == DOING_PING){
            answerPing();
        }
    }
    while(1);
}
void answerPing(){
    send(clientSocket, &Message, sizeof(Message), 0);
}
void countAndSend(){
    struct operation newOperation;

    newOperation.ID = atoi(strtok(Message.buffer, " "));
    newOperation.x = atoi(strtok(NULL, " "));
    newOperation.y = atoi(strtok(NULL, " "));
    newOperation.type = atoi(strtok(NULL, " \n"));

    printf("%lu %lu %lu %d\n", newOperation.ID, newOperation.x, newOperation.y, newOperation.type);

    int result;

    switch (newOperation.type){
        case ADD: result = newOperation.x + newOperation.y; break;
        case SUB: result = newOperation.x - newOperation.y; break;
        case MUL: result = newOperation.x * newOperation.y; break;
        case DIV: result = newOperation.x / newOperation.y; break;
        default: result = 0;
    }

    printf("Wynik = %d\n", result);
    Message.type = OPERATION_RESULT;
    sprintf(Message.buffer, "%lu %lu\n", newOperation.ID, result);
    send(clientSocket, &Message, sizeof(Message), 0);
}
void exitClient(){
    if (clientSocket != -1){
        Message.type = CLOSE_CLIENT;
        send(clientSocket, &Message, sizeof(Message), 0);

        sleep(1);
        shutdown(clientSocket, SHUT_RDWR);
        close(clientSocket);

    }
    printf("Bye!");
    exit(0);

}
void sendHello(){
    Message.type = NEW_CLIENT;
    strcpy(Message.buffer, name);
    send(clientSocket, &Message, sizeof(Message), 0);
    if(recv(clientSocket, &Message, sizeof(Message), 0 ) ==-1){
        fprintf(stderr, "Cannot recieve message\n");
        exit(1);
    }
    if (Message.type == NO_SPACE){
        fprintf(stderr, "Cannot connect because of lack of space\n");
        exit(1);
    }
    else if (Message.type == ADD_CLIENT_SUCCESS){
        printf("Success!!\n");
    }
    else if (Message.type == IS_CLIENT_WITH_THAT_NAME){
        fprintf(stderr, "Cannot connect because of existing client with that name\n");
        exit(1);
    }

}
void launchUnix() {
    clientSocket = socket(AF_UNIX, SOCK_STREAM, 0);

    if(socket == -1){
        fprintf(stderr, "Cannot open socket\n");
        exit(1);
    }
    struct sockaddr_un addressLocal;
    addressLocal.sun_family = AF_UNIX;
    strcpy(addressLocal.sun_path, address);
    if(connect(clientSocket, (struct sockaddr *) & addressLocal, sizeof(addressLocal)) == -1){
        fprintf(stderr, "Cannot connect on local\n");
        exit(1);
    }
    printf("Connected\n");
}

void launchWeb() {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket == -1){
        fprintf(stderr, "Cannot open webSocket\n");
        exit(1);
    }

    struct sockaddr_in webAddress;
    webAddress.sin_port = htons(port);
    webAddress.sin_family = AF_INET;

    if(inet_aton(address, &webAddress.sin_addr) == 0){ // laczenie poprzez ip
        fprintf(stderr, "Cannot connect via ip adress\n");
        exit(1);
    }

    if(connect(clientSocket, (struct sockaddr *) & webAddress, sizeof(webAddress)) == -1){
        fprintf(stderr, "Cannot connect by adress\n");
        exit(1);
    }
    printf("Connected\n");
}

bool parseArguments(int argc, char **argv) {
    if ((argc != 4) && (argc != 5)) return false;
    if (!isNumber(argv[2])) return false;
    if ((atoi(argv[2])) != 0 && (atoi(argv[2]) != 1)) return false;
    if ((atoi(argv[2]) == 1) && (argc != 5)) return false;

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