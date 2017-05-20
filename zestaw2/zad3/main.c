#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

bool parseArguments(int argc, char **argv);

void showInteractions(char *filePath);

void displayManual();

int getByte();

bool getVersion();

void setLockOnRead(char *path, int byte, bool waiting);

char getSwapChar();

void swapInFile(char *path, int byte, char swap);

void displayChar(char *path, int byte);

void setLockOnWrite(char *path, int byte, bool waiting);

void unlock(char *path, int byte,bool ifRead, bool ifWaiting);

bool getUnlockType();

void unlockRead(char *path, int byte, bool waiting);

void unlockWrite(char *path, int byte, bool waiting);

void displayLockList(char *path);

int main(int argc, char** argv){

    if(!parseArguments(argc, argv)){
        fprintf(stderr, "Invalid arguments\n");
        exit(1);
    }

    showInteractions(argv[1]);
    return 0;
}

void showInteractions(char *filePath) {
    char selection;
    int byte;
    bool ifWaiting;
    bool ifRead;
    char swap;

    do{
        printf("\033[H\033[J");
        displayManual();
        scanf("%s",&selection);

        switch(selection){
            case '0':
		printf("Bye bye!\n");
                return;
            case '1':
                byte = getByte();
                ifWaiting = getVersion();
                setLockOnRead(filePath, byte, ifWaiting);
                break;
            case '2':
                byte = getByte();
                ifWaiting = getVersion();
                setLockOnWrite(filePath, byte, ifWaiting);
                break;
            case '3':
                displayLockList(filePath);
                break;
            case '4':
                byte = getByte();
                ifRead = getUnlockType();
                ifWaiting = getVersion();
                unlock(filePath, byte, ifRead, ifWaiting);
                break;
            case '5':
                byte = getByte();
                displayChar(filePath, byte);
                break;
            case '6':
                byte = getByte();
                swap = getSwapChar();
                swapInFile(filePath, byte, swap);
                break;
            default:
                printf("Please select correct option\n");
                break;
        }

        printf("Press enter to continue\n");
        while(getchar()!='\n'); getchar();
    }
    while(selection);
}

void displayLockList(char *path) {
    struct flock *newFlock = malloc(sizeof(struct flock));

    int file = open(path, O_RDONLY);
    if (file == -1){// cannot open file
        fprintf(stderr, "Cannot open selected file!\n");
        exit(1);
    }

    int endOfFile = lseek(file, 0, SEEK_END); // patrzymy gdzie sie konczy plik (wskaznik na koniec pliku)
    int i = 0;

    do{
        lseek(file, i, 0); //przesuwamy na nastepny bajt

        newFlock->l_type = F_WRLCK; //jakakolwiek blokada
        newFlock->l_whence = 0;
        newFlock->l_start = i;
        newFlock->l_len = 1;

        fcntl(file, F_GETLK, newFlock);
        /*przekazujemy strukture opisujaca blokade, która (potencjalnie) chcielibysmy zalozyc, a w
        odpowiedzi nasza struktura zostaje wypelniona informacja o konfliktujacej blokadzie (PID procesu,
        zakres bajtow) lub informacja że zalozenie blokady byloby mozliwe (ustawienie F_UNLCK w polu
        l_type struktury)*/

        if(newFlock->l_type == F_RDLCK)
            printf("Locked on read byte %d ", i);
        else if (newFlock->l_type == F_WRLCK)
            printf("Locked on write byte %d ", i);
        if (newFlock->l_type != F_UNLCK)
            printf("by PID %d\n", newFlock->l_pid);

        i++;

    }
    while(i != endOfFile);
}

bool getUnlockType() {
    char version;
    printf("Unlock default read (any char) or write (W) ?\n");
    scanf("%s",&version);
    if (version == 'W') return true;
    else return false;

}

void unlock(char *path, int byte, bool ifRead, bool ifWaiting) {
    if (ifRead) unlockRead(path, byte, ifWaiting);
    else unlockWrite(path, byte, ifWaiting);
}

void unlockWrite(char *path, int byte, bool waiting) {
    int file = open(path, O_RDONLY); // do odczytu
    if (file == -1){// cannot open file
        fprintf(stderr, "Cannot open selected file!\n");
        exit(1);
    }

    struct flock *newFlock = malloc(sizeof(struct flock));

    newFlock->l_type = F_UNLCK ; // odblokuj
    newFlock->l_whence = 0; // wzgledem poczatku pliku
    newFlock->l_len = 1; // bo jeden bajt
    newFlock->l_start = byte; // byte to numer bajtu ktory mamy odblokowac, i jest to tez offset

    if(waiting){
        fcntl(file, F_SETLKW, newFlock); // F_SETLKW to flaga przez co czekamy na okazje
    }
    else{
        fcntl(file, F_SETLK, newFlock); // F_SETLK to flaga zwracajaca od razu -1
    }

}

void unlockRead(char *path, int byte, bool waiting) {
    int file = open(path, O_RDONLY); // do odczytu
    if (file == -1){// cannot open file
        fprintf(stderr, "Cannot open selected file!\n");
        exit(1);
    }

    struct flock *newFlock = malloc(sizeof(struct flock));

    newFlock->l_type = F_UNLCK; // czytanie moze byc dzielone
    newFlock->l_whence = 0; // wzgledem poczatku pliku
    newFlock->l_len = 1; // bo jeden bajt
    newFlock->l_start = byte; // byte to numer bajtu ktory mamy zablokowac, i jest to tez offset

    if(waiting){
        fcntl(file, F_SETLKW, newFlock); // F_SETLKW to flaga przez co czekamy na okazje
    }
    else{
        fcntl(file, F_SETLK, newFlock); // F_SETLK to flaga zwracajaca od razu -1
    }
}

void setLockOnWrite(char *path, int byte, bool waiting) {
    int file = open(path, O_WRONLY); // do odczytu
    if (file == -1){// cannot open file
        fprintf(stderr, "Cannot open selected file!\n");
        exit(1);
    }

    struct flock *newFlock = malloc(sizeof(struct flock));

    newFlock->l_type = F_WRLCK; // tylko do zapisu
    newFlock->l_whence = 0; // wzgledem poczatku pliku
    newFlock->l_len = 1; // bo jeden bajt
    newFlock->l_start = byte; // byte to numer bajtu ktory mamy zablokowac, i jest to tez offset

    if(waiting){
        fcntl(file, F_SETLKW, newFlock); // F_SETLKW to flaga przez co czekamy na okazje
    }
    else{
        fcntl(file, F_SETLK, newFlock); // F_SETLK to flaga zwracajaca od razu -1
    }

}

void displayChar(char *path, int byte) {
    char res[1];
    int file = open(path, O_RDONLY);

    if (file == -1){
        fprintf(stderr, "Cannot open selected file!\n");
        exit(1);
    }

    lseek(file, byte, 0);
    read(file, res, 1); //<- jeden znak
    printf("Selected char:\n");
    printf("%s\n", res);
}

void swapInFile(char *path, int byte, char swap) {
    char res[1];
    int file = open(path, O_RDWR); // read and write

    if (file == -1){
        fprintf(stderr, "Cannot open selected file!\n");
        exit(1);
    }
    lseek(file, byte, 0);

    res[0] = swap;
    write(file, res, sizeof(char));
}

char getSwapChar() {
    char swap;
    printf("Write character to swap:\n");
    scanf(" %c", &swap);
    return swap;
}

void setLockOnRead(char *path, int byte, bool waiting) {
    int file = open(path, O_RDONLY); // do odczytu
    if (file == -1){// cannot open file
        fprintf(stderr, "Cannot open selected file!\n");
        exit(1);
    }

    struct flock *newFlock = malloc(sizeof(struct flock));

    newFlock->l_type = F_RDLCK; // czytanie moze byc dzielone
    newFlock->l_whence = 0; // wzgledem poczatku pliku
    newFlock->l_len = 1; // bo jeden bajt
    newFlock->l_start = byte; // byte to numer bajtu ktory mamy zablokowac, i jest to tez offset

    if(waiting){
        fcntl(file, F_SETLKW, newFlock); // F_SETLKW to flaga przez co czekamy na okazje
    }
    else{
        fcntl(file, F_SETLK, newFlock); // F_SETLK to flaga zwracajaca od razu -1
    }

}

bool getVersion() {
    char version;
    printf("Write lock version: default with error (any char) or waiting (W) ?\n");
    scanf("%s",&version);
    if (version == 'W') return true;
    else return false;

}

int getByte() {
    int byte;
    printf("Which byte do you select?\n");
    scanf("%d",&byte);
    return byte;

}

void displayManual() {
    printf("Welcome in Gabrysia's lock super generator!\n");
    printf("Here are options:\n");
    printf("0 -> exit program\n");
    printf("1 -> set read lock on single char\n");
    printf("2 -> set write lock on single char\n");
    printf("3 -> list of locked chars\n");
    printf("4 -> unlock selected lock\n");
    printf("5 -> read selected char\n");
    printf("6 -> change selected char\n");

}

bool parseArguments(int argc, char **argv) {
    if (argc != 2) return false;

    int file = open(argv[1], O_RDONLY);
    if (file == -1) return false;
    close(file);
    return true;
}
