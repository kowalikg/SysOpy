#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>

void printTimes(struct tms *previousTms, clock_t *previousClock, bool start);
bool isNumber(char *string);
int stringToNumber(char *string);
bool parseArguments(int argc, char **args);
void generateFileSys(char *filePath, int records, int bytes);
void permutation(char * type, char * filePath, int records, int bytes);
void bubbleSort(char * type, char * filePath, int records, int bytes);
void sysPermutate(char* filePath, int records, int bytes);
void libPermutate(char* filePath, int records, int bytes);
void sysSort(char* filePath, int records, int bytes);
void libSort(char* filePath, int records, int bytes);
bool sysIsLater(int file, int p1, int p2, int bytes);
bool libIsLater(FILE* file, int p1, int p2, int bytes);
void sysSwapRecords(int file, int p1, int p2, int bytes);
void libSwapRecords(FILE* file, int p1, int p2, int bytes);

int main(int argc, char** argv){

    if (!parseArguments(argc,argv)){
        printf("Invalid arguments\n");
        exit(1);
    }

    struct tms previousTms;
    clock_t previousClock;
    printTimes(&previousTms, &previousClock, true);

    if(strcmp(argv[2], "generate") == 0) generateFileSys(argv[3], stringToNumber(argv[4]), stringToNumber(argv[5]));
    else if(strcmp(argv[2], "shuffle") == 0) permutation(argv[1], argv[3], stringToNumber(argv[4]), stringToNumber(argv[5]));
    else if(strcmp(argv[2], "sort") == 0) bubbleSort(argv[1], argv[3], stringToNumber(argv[4]), stringToNumber(argv[5]));

    printTimes(&previousTms, &previousClock, false);

    return 0;
}
void sysSort(char* filePath, int records, int bytes){
    printf("Sorting %d records (%d bytes) with syslibrary\n", records, bytes);
    int file = open(filePath, O_RDWR); // read and write
    if(file < 0){
        fprintf(stderr, "Cannot open file\n");
        exit(1);
    }
    for (int i = 0; i < records; i++){
        for (int j = 0; j < records - i - 1; j++){
            if (!sysIsLater(file, j, j+1, bytes)){
                sysSwapRecords(file, j, j+1, bytes);
            }

        }
    }
    close(file);

}
bool sysIsLater (int file, int p1, int p2, int bytes){
    char f1[bytes + sizeof(char)];
    char f2[bytes + sizeof(char)]; //ile bajtow + '\0'

    //przesuwamy wskaznik
    lseek(file, p1 * (bytes + 1) ,0); //<- 0 -> od poczatku pliku, (bytes+1) bo musimy juz wejsc na poczatek kolejnego
    //wczytujemy biezace polozenie wskaznika (bajty)
    read(file, f1, bytes + 1);
    //drugi plik
    lseek(file, p2 * (bytes + 1) ,0);
    read(file, f2, bytes + 1);
    //printf("%s ? %s\n", f1[0], f2[0]);
    if(f1[0] >= f2[0]) return false;
    else return true;
}
bool libIsLater(FILE* file, int p1, int p2, int bytes){
    char f1[bytes + sizeof(char)];
    char f2[bytes + sizeof(char)]; //ile bajtow + '\0'

    //przesuwamy wskaznik
    fseek(file, p1 * (bytes + 1) ,0); //<- 0 -> od poczatku pliku, (bytes+1) bo musimy juz wejsc na poczatek kolejnego
    //wczytujemy biezace polozenie wskaznika (bajty)
    fread(f1, sizeof(char), bytes + 1, file); // <- ile bytes o dlugosci sizeof(char) ma byc wczytancyh
    //drugi plik
    fseek(file, p2 * (bytes + 1) ,0);
    fread(f2, sizeof(char), bytes + 1, file);

    if(f1[0] >= f2[0]) return false;
    else return true;
}
void libSort(char* filePath, int records, int bytes){
    FILE * file = fopen(filePath, "r+"); //read and write
    if(file == NULL){
        fprintf(stderr, "Cannot open file\n");
        exit(1);
    }
    printf("Sorting %d records (%d bytes) with liblibrary\n", records, bytes);
    for (int i = 0; i < records; i++){
        for (int j = 0; j < records - i - 1; j++){
            if (!libIsLater(file, j, j+1, bytes)){
                libSwapRecords(file, j, j+1, bytes);
            }

        }
    }
    fclose(file);
}
void libPermutate(char* filePath, int records, int bytes) {
    FILE * file = fopen(filePath, "r+"); //read and write
    if(file == NULL){
        fprintf(stderr, "Cannot open file\n");
        exit(1);
    }
    int randomIndex;
    printf("Permutate %d records (%d bytes) with liblibrary\n", records, bytes);
    for (int i = records - 1; i > 0; i--){ //algorytm permutacji
        randomIndex = rand() % (i+1);
        libSwapRecords(file, i, randomIndex, bytes);
    }
    fclose(file);

}
void libSwapRecords(FILE* file, int p1, int p2, int bytes){

    char f1[bytes + sizeof(char)];
    char f2[bytes + sizeof(char)]; //ile bajtow + '\0'

    //przesuwamy wskaznik
    fseek(file, p1 * (bytes + 1) ,0); //<- 0 -> od poczatku pliku, (bytes+1) bo musimy juz wejsc na poczatek kolejnego
    //wczytujemy biezace polozenie wskaznika (bajty)
    fread(f1, sizeof(char), bytes + 1, file); // <- ile bytes o dlugosci sizeof(char) ma byc wczytancyh
    //drugi plik
    fseek(file, p2 * (bytes + 1) ,0);
    fread(f2, sizeof(char), bytes + 1, file);

    //teraz musimy znalezc wskaznik do f1 i wpisac f2 oraz znalezc wskaznik do f2 i wpisac f1
    fseek(file, p1 * (bytes + 1), 0);
    fwrite(f1, sizeof(char), bytes + 1, file);
    fseek(file, p2 * (bytes + 1), 0);
    fwrite(f2, sizeof(char), bytes + 1, file);

}
void sysSwapRecords(int file, int p1, int p2, int bytes){
    char f1[bytes + sizeof(char)];
    char f2[bytes + sizeof(char)]; //ile bajtow + '\0'

    //przesuwamy wskaznik
    lseek(file, p1 * (bytes + 1) ,0); //<- 0 -> od poczatku pliku, (bytes+1) bo musimy juz wejsc na poczatek kolejnego
    //wczytujemy biezace polozenie wskaznika (bajty)
    read(file, f1, bytes + 1);
    //drugi plik
    lseek(file, p2 * (bytes + 1) ,0);
    read(file, f2, bytes + 1);

    //teraz musimy znalezc wskaznik do f1 i wpisac f2 oraz znalezc wskaznik do f2 i wpisac f1
    lseek(file, p1 * (bytes + 1), 0);
    write(file, f2, bytes + 1);
    lseek(file, p2 * (bytes + 1), 0);
    write(file, f1, bytes + 1);
}
void sysPermutate(char* filePath, int records, int bytes){
    int file = open(filePath, O_RDWR); // read and write
    if(file < 0){
        fprintf(stderr, "Cannot open file or file doesnt exist\n");
        exit(1);
    }
    int randomIndex;
    printf("Permutate %d records (%d bytes) with syslibrary\n", records, bytes);
    for (int i = records - 1; i > 0; i--){ //algorytm permutacji
        randomIndex = rand() % (i+1);
        sysSwapRecords(file, i, randomIndex, bytes);

    }
    close(file);
}
void permutation(char * type, char * filePath, int records, int bytes){
    if (strcmp(type, "lib") == 0) libPermutate(filePath, records, bytes);
    else sysPermutate(filePath, records, bytes);

}
void bubbleSort(char * type, char * filePath, int records, int bytes){
    if (strcmp(type, "lib") == 0) libSort(filePath, records, bytes);
    else sysSort(filePath, records, bytes);
}
void generateFileSys(char* filePath, int records, int bytes) {

    int file = open(filePath, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IWOTH | S_IXOTH);
    int random = open("/dev/urandom", O_RDONLY);

    if(random < 0){
        fprintf(stderr, "Cannot create file\n");
        exit(1);
    }
    if(file < 0){
        fprintf(stderr, "Cannot create file\n");
        exit(1);
    }
    printf("Generate %d records (%d bytes)\n", records, bytes);
    char buffer;
    for (int r = 0; r < records; r++) {
        for (int b = 0; b < bytes; b++) {
            read(random, &buffer, sizeof(buffer));
            write(file, &buffer, sizeof(buffer));

        }
        buffer = '\n';
        write(file, &buffer, sizeof(buffer));

    }
    close(file);
    close(random);
}

bool parseArguments(int argc, char **args) {
    /*
     * correct order:
     * library: sys / lib
     * mode: generate / shuffle / sort
     * name of file: name
     * how many records: int
     * bytes: int
     */
    if (argc != 6) return false;
    if (strcmp(args[1], "sys") != 0 && strcmp(args[1], "lib") != 0) return false;
    if (strcmp(args[2], "generate") != 0 && strcmp(args[2], "shuffle") != 0 && strcmp(args[2], "sort") != 0) return false;
    if (!isNumber(args[4])) return false;
    if (!isNumber(args[5])) return false;

    return true;
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

bool isNumber(char *string) {
    int i = 0;
    while(string[i] != '\0'){
        if (string[i] >= '0' && string[i] <= '9') i++;
        else return false;
    }
    return true;
}

void printTimes(struct tms *previousTms, clock_t *previousClock, bool start) {
    struct tms currentTms;
    times(&currentTms);
    clock_t currentClock = clock();

    if(!start){
        printf("Real time:\t%.6f\n",((double) (currentClock - *(previousClock))) / CLOCKS_PER_SEC );
        printf("User time:\t%.6f\n",((double) (currentTms.tms_stime - previousTms->tms_stime)) / sysconf(_SC_CLK_TCK));
        printf("System time:\t%.6f\n",((double) (currentTms.tms_utime - previousTms->tms_utime)) / sysconf(_SC_CLK_TCK));
    }

    *previousTms = currentTms;
    *previousClock = currentClock;

}