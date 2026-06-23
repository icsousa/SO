#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc , char** argv) {

    int fdin = open("etc/passwd", O_RDONLY);
    int fdout = open("saida.txt", O_CREAT | O_TRUNC | O_WRONLY, 0666);
    int fderr = open("erros.txt", O_CREAT | O_TRUNC | O_WRONLY, 0666);

    dup2 (fdin, 0);
    close(fdin);

    dup2(fdout, 1);
    close(fdout);

    dup2(fderr, 2);
    close(fderr);

    printf("Terminei!");
    
    return 0;
}  