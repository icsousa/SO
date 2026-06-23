#include "guiao1.h"

int mycat (){

    char* buffer = (char*)malloc(1024);

    ssize_t bytes_read = 0;
    ssize_t bytes_written = 0;

    while ((bytes_read = read(0,buffer, 1024))>0) {
        bytes_written += write(1,buffer, bytes_read);
    }

    printf ("Bytes read: %d, Bytes written: %d\n", bytes_read, bytes_written);

    free(buffer);
    return 0;
}

int mycat_adv (const char* path) {

    char* buffer = (char*)malloc(1024);

    int fd = open(path, O_RDONLY);
    // int fd = open(path , O_CRATE|O_WRONLY|O_TRUNV, 0666);

    if (fd == -1){
        perror ("Open error");
        return -1;
    }

    ssize_t bytes_read = 0;
    ssize_t bytes_written = 0;

    
    while ((bytes_read = read(fd, buffer, 1024))>0) {
        bytes_written += write(1, buffer, bytes_read);
    }

    printf ("Bytes read: %d, Bytes written: %d\n", bytes_read, bytes_written);

    free(buffer);
    return 0;

}

int mycp (const char* path, const char* path1, const char* path2) {
    int buffer_size = atoi(path2);
    char* buffer = (char*)malloc(buffer_size);

    int fd = open(path, O_RDONLY);
    int fd1 = open(path1, O_CREAT|O_WRONLY| O_TRUNC, 0666);

    if (fd == -1){
        perror ("Open error");
        return -1;
    }

    if (fd1 == -1){
        perror ("Open error");
        return -1;
    }

    ssize_t bytes_read = 0;
    ssize_t bytes_written = 0;

    
    while ((bytes_read = read(fd, buffer, buffer_size))>0) {
        bytes_written += write(fd1, buffer, bytes_read);
    }

    free(buffer);

    close(fd1);
    close(fd);

    return 0;


}