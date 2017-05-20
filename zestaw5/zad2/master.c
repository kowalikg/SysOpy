//
// Created by gaba on 4/19/17.
//

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <complex.h>
#include <unistd.h>

struct mapPoint{
    int x;
    int y;
    int value;
};
bool parseArguments(int amount, char **arguments);
bool isNumber(char *string);
int convertToMap(double value, int minValue, int maxValue, int size);
int ** generateMap(int size);
void launchSlave(char * path);
struct mapPoint generatePoint(char* line, int size);
void saveToFile(int ** map, int size);
void launchGnyPlot(int size);

int main(int argc, char *argv[]) {
    if (!parseArguments(argc, argv)) {
        fprintf(stderr, "Invalid arguments\n");
        exit(1);
    }

    char pipePath[100];
    strcpy(pipePath, argv[1]);

    int R = atoi(argv[2]);

    if (mkfifo(pipePath, 0666)){ // do zapisu i odczytu
        fprintf(stderr, "Cannot launch pipe\n");
        exit(1);
    }
    printf("%s\n", pipePath);

    int ** map = generateMap(R);

    pid_t forked = fork();

    if (forked == -1){
        fprintf(stderr, "Cannot fork process\n");
        exit(1);
    }
    else if (forked == 0){
        launchSlave(pipePath);
        exit(0);
    }
    else {
        sleep(1);

        char buffer[100];
        memset(buffer, 0, 100);
        int pipe = open(pipePath, O_RDONLY);

        int counter = 1;

        while(read(pipe, buffer, 100) > 0){
            printf("Reading line: %d / 10000000\n", counter++);
            struct mapPoint point = generatePoint(buffer, R);
            map[point.x][point.y] = point.value;
        }
        close(pipePath);
        unlink(pipePath);

        saveToFile(map, R);
        launchGnuPlot(R);

    }
}
void launchGnuPlot(int size){
    FILE* gnuPlot = popen("gnuplot", "w");

    if (!gnuPlot){
        fprintf(stderr, "Cannot launch gnuplot\n");
        exit(1);
    }
    fprintf(gnuPlot, "set view map\n");
    fprintf(gnuPlot, "set xrange[0:%d]\n", size);
    fprintf(gnuPlot, "plot 'data' with image\n");

    fflush(gnuPlot);
    getchar();

    pclose(gnuPlot);
}
void saveToFile(int ** map, int size){
    FILE* dataFile = fopen("data", "w");

    for (int x = 0; x < size; x++)
        for (int y = 0; y < size; y++) fprintf(dataFile, "%d %d %d\n", x, y, map[x][y]);
    fclose(dataFile);
}
struct mapPoint generatePoint(char* line, int size){
    struct mapPoint point;

    char * pointer = strtok(line, "|");
    double re = atof(pointer);

    pointer = strtok(NULL, "|");
    double im = atof(pointer);

    pointer = strtok(NULL, "\n");
    int it = atoi(pointer);

    point.x = convertToMap(re, -2, 1, size);
    point.y = convertToMap(im, -1, 1, size);
    point.value = it;

    return point;

}
void launchSlave(char * path){
    char* slaveArgs[5] = {"slave", path, "10000000", "100", 0};
    if (execv(slaveArgs[0], slaveArgs) == -1){
        fprintf(stderr, "Cannot launch slave\n");
        exit(1);
    }
    printf("Slave succesfully launched\n");
}
int ** generateMap(int size){
    int ** map = (int **) malloc(size * sizeof(int*));

    int i = 0;
    while(i < size) map[i++] = (int *) malloc(size * sizeof(int));

    return map;
}
int convertToMap(double value,int minValue, int maxValue, int size){
    return (int)(((double)(value - minValue) / (double)(maxValue - minValue)) * size);
}
bool parseArguments(int amount, char **arguments){
    if (amount != 3) return false;
    if (!isNumber(arguments[2])) return false;
    return true;
}
bool isNumber(char *string) {

    int i = 0;
    while(string[i] != '\0'){
        if (string[i] >= '0' && string[i] <= '9') i++;
        else return false;
    }
    return true;
}
