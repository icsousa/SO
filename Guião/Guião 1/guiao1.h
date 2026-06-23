#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct person {
    char name[200];
    int age;
} Person;

int mycat ();

int mycat_adv (const char* path);

int mycp(const char* path, const char* path1, const char* path2);