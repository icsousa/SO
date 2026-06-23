CC = gcc
CFLAGS = -Wall -g

all: guiao1

guiao1: guiao1.o main.o
	$(CC) $(CFLAGS) -o mycat guiao1.o main.o

guiao1.o: guiao1.c guiao1.h
	$(CC) $(CFLAGS) -c guiao1.c

main.o: main.c guiao1.h
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f *.o guiao1




