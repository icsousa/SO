#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    int status;
    pid_t pid = fork();

    if (pid == 0) {
        execlp("ls", "ls", "-l", NULL);
        _exit(-1);
    } else {
        wait(&status);
        printf("Executado no processo %d com status %d\n", pid, WEXITSTATUS(status));
    }

    return 0;
}