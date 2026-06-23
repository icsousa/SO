void pai_to_filho() {
    int fd[2];
    int res = pipe (fd);
    int status;

    if (res==-1) {perror("Erro na criação do pipe");}

    pid_t pid == fork();

    if(pid == 0) { //filho
        close(fd[1]);
        int value_to_read = 0;
        
        res = read(fd[0], &value_to_read, sizeof(int));
        printf("Filho leu %d bytes do pipe (%d)\n", res, value_to_read);
        close(fd[0]);

        _exit(0);
    }
    else { //pai
        close(fd[0]);
        int value = 10;
        
        res = write(fd[1], &value, sizeof(int));
        printf("Pai escreveu %d\n", res);
        close (fd[1]);

        wait(NULL);

        if (WIFEXITED(status)) {printf("Terminou %d\n", WEXISTATUS(status));}
    }
}

int main(int argc, int *argv[]) {

}