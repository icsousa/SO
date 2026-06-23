#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>

#define NUM_LINHAS 10
#define NUM_COLS 10

int main() {
    int status, find = 10;
    int mat[NUM_LINHAS][NUM_COLS];
    pid_t pid, matches[NUM_LINHAS];

    for (int i = 0; i < NUM_LINHAS; i++) {
        if (fork() == 0) {
            for (int j = 0; j < NUM_COLS; j++)
                if (mat[i][j] == find)
                    _exit(i);
            
            _exit(-1);
        }
    }

    for (int i = 0; i < NUM_LINHAS; i++) {
        pid = wait(&status);
        int linha = WEXITSTATUS(status);
        if (linha != 255)
            matches[linha] = pid;
    }

    for (int i = 0; i < NUM_LINHAS; i++) {
        for (int j = 0; j < NUM_COLS; j++)
            printf("%2d ", mat[i][j]);
        
        printf("\n");
    }

    printf("\n\n");

    for (int i = 0; i < NUM_LINHAS; i++)
        if (matches[i])
            printf("Encontrado na linha %d\n", i);

    return 0;
}