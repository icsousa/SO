#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>


int main() {
    char buffer[32];
    ssize_t n_read;

    int pipe_fds[2];
    pipe(pipe_fds);
    int read_fd = pipe_fds[0];
    int write_fd = pipe_fds[1];

    if (fork() == 0) {
        close(write_fd);
        dup2(read_fd, STDIN_FILENO);
        close(read_fd);

        execlp("wc", "wc", "-w", NULL);
        _exit(EXIT_FAILURE);
    }

    close(read_fd);

    // Temos extremidade de escrita do pipe.
    char message[] = "O pai está a escrever para o filho\n";
    write(write_fd, message, sizeof(message));

    sleep(2);
    write(write_fd, "Está à espera\n", sizeof("Está à espera\n"));

    close(write_fd);
    wait(NULL);

    return EXIT_SUCCESS;
}