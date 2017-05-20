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
#include <ftw.h>


bool parseArguments(int argc, char **pString);

bool isNumber(char *string);

int stringToNumber(char *string);

char* generatePath(char *string);

void openDirectory(char *path, int size);

void generatePermissons(char permissions[9], struct stat stat);

void displayFileDetails(struct stat pDirent, const char* name, const char * perm);

int fun(const char *fpath, const struct stat *sb, int tflag);

int size;

int main(int argc, char** argv) {

    if(!parseArguments(argc, argv)) {
        fprintf(stderr,"Not valid arguments\n"); //stderr is a standard output error
        exit(1);
    }
    size = stringToNumber(argv[2]);

    char * absPath = generatePath(argv[1]);

    if(!absPath){
        fprintf(stderr,"Not valid filepath\n"); //stderr is a standard output error
        exit(1);
    }

    ftw(absPath, fun, FTW_F);

    return 0;
}

int fun(const char *fpath, const struct stat *sb, int tflag){
   if (!sb) return 0;
   if(S_ISREG(sb->st_mode) && (int) sb->st_size < size){//check if regular file and if size is less than max size launched in program -> if yes than display stats
        char permissions[10];

        generatePermissons(permissions, *sb);
        displayFileDetails(*sb, fpath, permissions);
   }
   return 0;
}



void displayFileDetails(struct stat pDirent, const char* name, const char* perm) {
    printf("Name:%s\nsize:%d\nperm:%s\nlast modif:%s", name, (int) pDirent.st_size, perm, ctime(&pDirent.st_mtim));

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
//
// Created by gaba on 3/21/17.
//

