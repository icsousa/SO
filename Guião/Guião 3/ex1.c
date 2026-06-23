#include <stdio.h>
#include <unistd.h>

int main(int agrc, char** argv) {
    int ret = 0;

    ret = execlp("ls", "ls", "-l", NULL);

    printf("Valor: %d\n", ret);
    return 0;
}