#if !defined(INCLUDES)
#define INCLUDES

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>

#define MAX_TITLE 200
#define MAX_AUTHORS 200
#define MAX_PATH 64
#define MAX_MSG_SIZE 512
#define MAX_DOCS 2000
#define MAX_CACHE 1000
#define FIFO_PATH "/tmp/dserver_fifo"
#define DEBUG 1

typedef struct {
    int id;
    char title[MAX_TITLE];
    char authors[MAX_AUTHORS];
    int year;
    char path[MAX_PATH];
} Document;

Document documentos[MAX_DOCS];
int num_docs = 0;

typedef struct {
    int id;
    int acessos;
    int h;
    Document doc;
} CacheEntry;

CacheEntry cache[MAX_CACHE];
int cache_size;
int cache_count;

extern char doc_folder[128];

#endif