#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <values.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "limits.h"


bool parseArguments(int argc, char **pString);

bool isNumber(char *string);

int stringToNumber(char *string);

char* generatePath(char *string);

void openDirectory(char *path, int size);

void generatePermissons(char permissions[9], struct stat stat);

void displayFileDetails(struct stat pDirent, char* name, char string1[10]);


int main(int argc, char** argv) {

    if(!parseArguments(argc, argv)) {
        fprintf(stderr,"Not valid arguments\n"); //stderr is a standard output error
        exit(1);
    }
    int size = stringToNumber(argv[2]);
    char * absPath = generatePath(argv[1]);

    if(!absPath){
        fprintf(stderr,"Not valid filepath\n"); //stderr is a standard output error
        exit(1);
    }

    openDirectory(absPath, size);
    free(absPath);

    return 0;
}

void openDirectory(char *rootPath, int size) {
    DIR *rootDirectory = opendir(rootPath);

    if(!rootDirectory){ //cannot open directory
        fprintf(stderr, "Cannot open directory\n");
        exit(1);
    }

    //struct dirent *readdir(DIR *dirp) <- open next record in directory

    struct dirent * currentFile = NULL;
    struct stat currentFileStatus;
    char fullPathName[PATH_MAX];

    do {
        currentFile = readdir(rootDirectory);
        if(currentFile) {
            snprintf(fullPathName, PATH_MAX, "%s/%s", rootPath, currentFile->d_name);
            if (lstat(fullPathName, &currentFileStatus) == -1){ //get file status -> int lstat(const char *path, struct stat *buf);
                fprintf(stderr, "Cannot get statistics\n");
                exit(1);
            }
            if(S_ISREG(currentFileStatus.st_mode) && (int) currentFileStatus.st_size < size){//check if regular file and if size is less than max size launched in program -> if yes than display stats
                char permissions[10];

                generatePermissons(permissions, currentFileStatus);
                displayFileDetails(currentFileStatus, fullPathName, permissions);

            }
            else if (S_ISDIR(currentFileStatus.st_mode) && (strcmp(currentFile->d_name, ".") != 0)
            && (strcmp(currentFile->d_name, "..") != 0)){

                snprintf(fullPathName, PATH_MAX, "%s/%s", rootPath, currentFile->d_name);

                openDirectory(fullPathName, size);

            }
        }

    }
    while(currentFile);
    if(closedir(rootDirectory) == -1) {
        fprintf(stderr,"Cannot close directory!\n");
        exit(1);
    }

}


void displayFileDetails(struct stat pDirent, char* name, char string1[10]) {
    printf("Name:%s\nsize:%d\nperm:%s\nlast modif:%s", name, (int) pDirent.st_size, string1, ctime(&pDirent.st_mtim));

}

void generatePermissons(char permissions[], struct stat stat) {
    //stat contain permissons about file status
    permissions[0] = stat.st_mode & S_IRUSR ? 'r' : '-';
    permissions[1] = stat.st_mode & S_IWUSR ? 'w' : '-';
    permissions[2] = stat.st_mode & S_IXUSR ? 'x' : '-';
    permissions[3] = stat.st_mode & S_IRGRP ? 'r' : '-';
    permissions[4] = stat.st_mode & S_IWGRP ? 'w' : '-';
    permissions[5] = stat.st_mode & S_IXGRP ? 'x' : '-';
    permissions[6] = stat.st_mode & S_IROTH ? 'r' : '-';
    permissions[7] = stat.st_mode & S_IWOTH ? 'w' : '-';
    permissions[8] = stat.st_mode & S_IXOTH ? 'x' : '-';
    permissions[9] = '\0';

}

char* generatePath(char *path) {
    char* absPath = malloc(sizeof(char)*PATH_MAX);
    absPath = realpath(path,absPath); // realpath() expands all symbolic links and resolves references to /./,/../ and extra '/' characters in the null-terminated string named by path to produce a canonicalized absolute pathname.
    return absPath;
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
bool parseArguments(int argc, char **pString) {
    if (argc != 3) return false;  //argv[0] is a name of program, next are arguments
    if(!isNumber(pString[2])) return false;
    return true;
}
