#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ARGS 10

int mysystem(char *command) {
    int status, i = 0;
    char *token, *args[MAX_ARGS];

    for (token = strtok(command, " "); token != NULL; token = strtok(NULL, " ")) {
        args[i++] = token;
    }

    args[i] = NULL;

    if (fork() == 0) {
        execvp(args[0], args);
        _exit(-1);
    } else {
        wait(&status);
        return WEXITSTATUS(status);
    }
}


int main(int argc, char* argv[]) {
    if (argc < 2)
        return EXIT_FAILURE;
    
    printf("status %d\n", mysystem(argv[1]));

    return 0;
}