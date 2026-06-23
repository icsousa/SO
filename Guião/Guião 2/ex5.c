#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>

int main() {
    int status, find = 10;
    int mat[10][50];
    pid_t pid;

    for (int i = 0; i < 10; i++) {
        if (fork() == 0) {
            for (int j = 0; j < 50; j++)
                if (mat[i][j] == find)
                    _exit(i);
            
            _exit(-1);
        }
    }

    for (int i = 0; i < NUM_LINHAS; i++) {
        pid = wait(&status);
        int linha = WEXITSTATUS(status);
        if (linha != -1)
            printf("Encontrado na linha %d\n", linha);
    }

    return 0;
}