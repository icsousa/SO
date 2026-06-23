#include "includes.h"

char doc_folder[128] = "./";
int access_history[MAX_DOCS] = {0};

void init_cache() {
    for (int i = 0; i < cache_size; i++) {
        cache[i].h = 0;
    }
    cache_count = 0;
}


Document* get_from_cache(int id) {
    for (int i = 0; i < cache_size; i++) {
        if (cache[i].h && cache[i].id == id) {
            cache[i].acessos++;
            access_history[id]++;
            #if DEBUG
                printf("[CACHE HIT] Document %d -> %d\n", id, cache[i].acessos);
            #endif
            return &cache[i].doc;
        }
    }
    #if DEBUG
        printf("[CACHE MISS] Document %d\n", id);
    #endif
    return NULL;
}


void add_to_cache(Document d) {
    for (int i = 0; i < cache_size; i++) {
        if (cache[i].h && cache[i].id == d.id) {
            cache[i].doc = d;
            cache[i].acessos++;
            access_history[d.id]++;
            #if DEBUG
                printf("[CACHE UPDATE] Document %d -> %d\n", d.id, cache[i].acessos);
            #endif
            return;
        }
    }

    for (int i = 0; i < cache_size; i++) {
        if (!cache[i].h) {
            access_history[d.id]++;
            cache[i].id = d.id;
            cache[i].doc = d;
            cache[i].acessos = access_history[d.id];
            cache[i].h = 1;
            cache_count++;
            #if DEBUG
                printf("[CACHE ADD] Document %d -> %d\n", d.id, cache[i].acessos);
            #endif
            return;
        }
    }

    int idx_menos_usado = -1;
    int min_acessos = __INT_MAX__;
    for (int i = 0; i < cache_size; i++) {
        if (cache[i].h && cache[i].acessos < min_acessos) {
            min_acessos = cache[i].acessos;
            idx_menos_usado = i;
        }
    }

    if (idx_menos_usado != -1) {
        #if DEBUG
            int id_removed = cache[idx_menos_usado].id, acessos_removed = cache[idx_menos_usado].acessos;
        #endif
        
        access_history[d.id]++;
        cache[idx_menos_usado].id = d.id;
        cache[idx_menos_usado].doc = d;
        cache[idx_menos_usado].acessos = access_history[d.id];
        cache[idx_menos_usado].h = 1;

        #if DEBUG
            printf("[CACHE REPLACE] - Document %d -> %d | + Document %d -> %d\n",
                   id_removed, acessos_removed, d.id, cache[idx_menos_usado].acessos);
        #endif
    }
}

void remove_from_cache(int id) {
    for (int i = 0; i < cache_size; i++) {
        if (cache[i].h && cache[i].id == id) {
            cache[i].h = 0;
            cache_count--;
            #if DEBUG
                printf("[CACHE REMOVE] Docuemnt %d removed manualy\n", id);
            #endif
            return;
        }
    }
}


int next_id = 1;

void handle_add(char *msg) {
    char *rest = msg;

    strtok_r(rest, "|", &rest);

    char *title = strtok_r(NULL, "|", &rest);
    char *authors = strtok_r(NULL, "|", &rest);
    char *year_str = strtok_r(NULL, "|", &rest);
    char *path = strtok_r(NULL, "|", &rest);

    char *pid_str = strrchr(path, ':');
    if (!pid_str) {
        #if DEBUG
            printf("Error: PID não encontrado na mensagem ADD.\n");
        #endif
        return;
    }

    *pid_str = '\0';
    pid_str++;        

    int year = atoi(year_str);
    int pid = atoi(pid_str);

    char full_path[128];
    snprintf(full_path, sizeof(full_path), "%s%s", doc_folder, path);

    for (int i = 0; i < num_docs; i++) {
        if (strcmp(documentos[i].path, full_path) == 0) {
            char client_fifo[64];
            snprintf(client_fifo, sizeof(client_fifo), "/tmp/client_fifo_%d", pid);
            int fd = open(client_fifo, O_WRONLY);
            if (fd >= 0) {
                char resposta[256];
                snprintf(resposta, sizeof(resposta), "Error: Document already indexed.\n");
                write(fd, resposta, strlen(resposta) + 1);
                close(fd);
            }
            #if DEBUG
                printf("[ADD BLOCKED] Document already exists: %s\n", full_path);
            #endif
            return;
        }
    }

    Document doc;
    doc.id = next_id++;
    strncpy(doc.title, title, sizeof(doc.title));
    strncpy(doc.authors, authors, sizeof(doc.authors));
    doc.year = year;
    strncpy(doc.path, full_path, sizeof(doc.path) - 1);
    doc.path[sizeof(doc.path) - 1] = '\0';

    if (num_docs < MAX_DOCS) {
        documentos[num_docs++] = doc;
        add_to_cache(doc);
    } else {
        #if DEBUG
            printf("Limite de documentos atingido.\n");
        #endif
    }

    char client_fifo[64];
    snprintf(client_fifo, sizeof(client_fifo), "/tmp/client_fifo_%d", pid);

    int fd, tentativas = 0;
    char resposta[512];
    snprintf(resposta, sizeof(resposta), "Document %d indexed.\n", doc.id);

    while ((fd = open(client_fifo, O_WRONLY)) < 0 && tentativas++ < 5) {
        usleep(100000);
    }

    if (fd >= 0) {
        write(fd, resposta, strlen(resposta) + 1);
        close(fd);
    } else {
        #if DEBUG
            printf("Erro ao abrir FIFO do cliente (%s) após tentativas.\n", client_fifo);
        #endif
    }
}

void handle_consult(char *msg) {
    char *rest = msg + 8;
    char *id_str = strtok(rest, ":");
    char *pid_str = strtok(NULL, ":");

    int id = atoi(id_str);
    int pid = atoi(pid_str);

    extern Document documentos[MAX_DOCS];
    extern int num_docs;

    Document *doc = get_from_cache(id);
    if (!doc) {
        for (int i = 0; i < num_docs; i++) {
            if (documentos[i].id == id) {
                doc = &documentos[i];
                add_to_cache(*doc);
                break;
            }
        }
    }

    char client_fifo[64];
    snprintf(client_fifo, sizeof(client_fifo), "/tmp/client_fifo_%d", pid);

    char resposta[512];
    if (doc) {
        snprintf(resposta, sizeof(resposta),
                 "Title: %s\nAuthors: %s\nYear: %d\nPath: %s\n",
                 doc->title, doc->authors, doc->year, doc->path);
    } else {
        snprintf(resposta, sizeof(resposta), "Document %d not found.\n", id);
    }

    int fd, tentativas = 0;
    while ((fd = open(client_fifo, O_WRONLY)) < 0 && tentativas++ < 5) {
        usleep(100000);
    }

    if (fd >= 0) {
        write(fd, resposta, strlen(resposta) + 1);
        close(fd);
    } else {
        #if DEBUG
            printf("Erro ao abrir FIFO do cliente (%s) após tentativas.\n", client_fifo);
        #endif
    }
}

void handle_delete(char *msg) {
    char *rest = msg + 7;
    char *id_str = strtok(rest, ":");
    char *pid_str = strtok(NULL, ":");

    int id = atoi(id_str);
    int pid = atoi(pid_str);

    int found = 0;
    for (int i = 0; i < num_docs; i++) {
        if (documentos[i].id == id) {
            for (int j = i; j < num_docs - 1; j++) {
                documentos[j] = documentos[j + 1];
            }
            remove_from_cache(id);
            num_docs--;
            found = 1;
            break;
        }
    }

    char client_fifo[64];
    snprintf(client_fifo, sizeof(client_fifo), "/tmp/client_fifo_%d", pid);

    char resposta[512];
    if (found) {
        snprintf(resposta, sizeof(resposta), "Index entry %d deleted.\n", id);
    } else {
        snprintf(resposta, sizeof(resposta), "Index entry %d not found.\n", id);
    }

    int fd, tentativas = 0;
    while ((fd = open(client_fifo, O_WRONLY)) < 0 && tentativas++ < 5) {
        usleep(100000);
    }

    if (fd >= 0) {
        write(fd, resposta, strlen(resposta) + 1);
        close(fd);
    } else {
        #if DEBUG
            printf("Erro ao abrir FIFO do cliente (%s) após tentativas.\n", client_fifo);
        #endif
    }
}

void handle_lines(char *msg) {
    char *rest = msg + 6;

    char *id_str = strtok(rest, ":");
    char *keyword = strtok(NULL, ":");
    char *pid_str = strtok(NULL, ":");

    int id = atoi(id_str);
    int pid = atoi(pid_str);

    Document *doc = get_from_cache(id);
    if (!doc) {
        for (int i = 0; i < num_docs; i++) {
            if (documentos[i].id == id) {
                doc = &documentos[i];
                add_to_cache(*doc);
                break;
            }
        }
    }

    char resposta[512];
    
    if (!doc) {
        snprintf(resposta, sizeof(resposta), "Document %d not found.\n", id);
    } else {
        int pipe_grep[2];
        int pipe_wc[2];

        if (pipe(pipe_grep) == -1 || pipe(pipe_wc) == -1) {
            perror("Error building pipes");
            snprintf(resposta, sizeof(resposta), "Error (pipes).");
        } else {
            pid_t pid1 = fork();
            if (pid1 == 0) {
                
                dup2(pipe_grep[1], 1);
                close(pipe_grep[0]);
                close(pipe_grep[1]);
                close(pipe_wc[0]);
                close(pipe_wc[1]);

                execlp("grep", "grep", keyword, doc->path, NULL);
                perror("Error excuting grep");
                exit(1);
            }

            pid_t pid2 = fork();
            if (pid2 == 0) {
                
                dup2(pipe_grep[0], 0);
                dup2(pipe_wc[1], 1);
                close(pipe_grep[0]);
                close(pipe_grep[1]);
                close(pipe_wc[0]);
                close(pipe_wc[1]);

                execlp("wc", "wc", "-l", NULL);
                perror("Error excuting wc");
                exit(1);
            }

            
            close(pipe_grep[0]);
            close(pipe_grep[1]);
            close(pipe_wc[1]);

            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);

            int n = read(pipe_wc[0], resposta, sizeof(resposta) - 1);
            close(pipe_wc[0]);

            if (n <= 0) {
                snprintf(resposta, sizeof(resposta), "0");
            } else {
                resposta[n] = '\0';
            }
        }
    }

    char client_fifo[64];
    snprintf(client_fifo, sizeof(client_fifo), "/tmp/client_fifo_%d", pid);
    
    int fd, tentativas = 0;
    while ((fd = open(client_fifo, O_WRONLY)) < 0 && tentativas++ < 5) {
        usleep(100000);
    }

    if (fd >= 0) {
        write(fd, resposta, strlen(resposta) + 1);
        close(fd);
    } else {
        #if DEBUG
            printf("Erro ao abrir FIFO do cliente (%s) após tentativas.\n", client_fifo);
        #endif
    }
}

void handle_search(char *msg) {
    char *rest = msg + 7; 
    char *keyword = strtok(rest, ":");
    char *pid_str = strtok(NULL, ":");

    int pid = atoi(pid_str);
    char resultados[2048] = "[";
    int primeira = 1;

    for (int i = 0; i < num_docs; i++) {
        Document *doc = get_from_cache(documentos[i].id);
        
        if (!doc) {
            doc = &documentos[i];
            add_to_cache(*doc);
        }

        
        pid_t grep_pid = fork();
        if (grep_pid == 0) {
            
            int null_fd = open("/dev/null", O_WRONLY);
            dup2(null_fd, 1);
            dup2(null_fd, 2);
            close(null_fd);

            execlp("grep", "grep", "-q", keyword, doc->path, NULL);
            perror("grep");
            exit(1);
        }

        int status;
        waitpid(grep_pid, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            
            if (!primeira) strcat(resultados, ", ");
            char id_str[10];
            sprintf(id_str, "%d", doc->id);
            strcat(resultados, id_str);
            primeira = 0;
        }
    }

    strcat(resultados, "]\n");

    
    char client_fifo[64];
    snprintf(client_fifo, sizeof(client_fifo), "/tmp/client_fifo_%d", pid);
    int fd = open(client_fifo, O_WRONLY);
    if (fd >= 0) {
        write(fd, resultados, strlen(resultados) + 1);
        close(fd);
    } else {
        perror("Error opening client FIFO\n");
    }
}

void handle_search_concurrent(char *msg) {
    char *rest = msg + 11;
    char *keyword = strtok(rest, ":");
    char *n_str = strtok(NULL, ":");
    char *pid_str = strtok(NULL, ":");
    int n_procs = atoi(n_str);
    int client_pid = atoi(pid_str);

    if (n_procs <= 0) n_procs = 1;
    if (n_procs > num_docs) n_procs = num_docs;

    int docs_per_proc = num_docs / n_procs;
    int extra = num_docs % n_procs;

    int pipe_fd[2];
    pipe(pipe_fd);

    for (int i = 0, start = 0; i < n_procs; i++) {
        int count = docs_per_proc + (i < extra ? 1 : 0);
        int begin = start;
        int end = start + count;
        start = end;

        pid_t pid = fork();
        if (pid == 0) {
            close(pipe_fd[0]);
            for (int j = begin; j < end; j++) {
                Document *doc = get_from_cache(documentos[j].id);
                if (!doc) {
                    doc = &documentos[j];
                    add_to_cache(*doc);
                }

                pid_t grep_pid = fork();
                if (grep_pid == 0) {
                    int null_fd = open("/dev/null", O_WRONLY);
                    dup2(null_fd, 1);
                    dup2(null_fd, 2);
                    close(null_fd);
                    execlp("grep", "grep", "-q", keyword, doc->path, NULL);
                    exit(1);
                }

                int status;
                waitpid(grep_pid, &status, 0);

                if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                    char id_str[16];
                    snprintf(id_str, sizeof(id_str), "%d\n", doc->id);
                    write(pipe_fd[1], id_str, strlen(id_str));
                }
            }
            close(pipe_fd[1]);
            exit(0);
        }
    }

    close(pipe_fd[1]);
    char resultados[2048] = "[";
    char buffer[1024];
    int primeira = 1;
    
    int total_lido;
    while ((total_lido = read(pipe_fd[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[total_lido] = '\0';
    
        char *linha = strtok(buffer, "\n");
        while (linha != NULL) {
            if (!primeira) strncat(resultados, ", ", sizeof(resultados) - strlen(resultados) - 1);
            strncat(resultados, linha, sizeof(resultados) - strlen(resultados) - 1);
            primeira = 0;
            linha = strtok(NULL, "\n");
        }
    }
    close(pipe_fd[0]);
    

    strncat(resultados, "]\n", sizeof(resultados) - strlen(resultados) - 1);

    for (int i = 0; i < n_procs; i++) {
        wait(NULL);
    }

    char client_fifo[64];
    snprintf(client_fifo, sizeof(client_fifo), "/tmp/client_fifo_%d", client_pid);

    int fd, tentativas = 0;
    while ((fd = open(client_fifo, O_WRONLY)) < 0 && tentativas++ < 5) {
        usleep(100000);
    }

    if (fd >= 0) {
        write(fd, resultados, strlen(resultados) + 1);
        close(fd);
    } else {
        perror("Erro ao abrir FIFO do cliente");
    }
}

void handle_shutdown(char *msg) {
    char *rest = msg + 9;
    int pid = atoi(rest);

    #if DEBUG
        printf("[SERVER] Closing at the request of the client PID %d...\n", pid);
    #endif

    char client_fifo[64];
    snprintf(client_fifo, sizeof(client_fifo), "/tmp/client_fifo_%d", pid);
    int fd = open(client_fifo, O_WRONLY);
    if (fd >= 0) {
        char resposta[] = "Server is shuting down.\n";
        write(fd, resposta, strlen(resposta) + 1);
        close(fd);
    } else {
        perror("Error opening client FIFO");
    }
}

int main(int argc, char *argv[]) {
    int running = 1;
    if (argc != 3) {
        printf("Use: %s \"document_folder\" \"cache_size\"\n", argv[0]);
        return 1;
    }

    strncpy(doc_folder, argv[1], sizeof(doc_folder) - 2);
    doc_folder[sizeof(doc_folder) - 1] = '\0';

    if (doc_folder[strlen(doc_folder) - 1] != '/') {
        strncat(doc_folder, "/", sizeof(doc_folder) - strlen(doc_folder) - 1);
    }

    cache_size = atoi(argv[2]);

    char buffer[MAX_MSG_SIZE];
    mkfifo(FIFO_PATH, 0666);

    #if DEBUG
        printf("[SERVER] Started with cache of %d documents.\n", cache_size);
        printf("[SERVER] Base directory of documents: %s\n", doc_folder);
        printf("[SERVER] Listening on %s...\n", FIFO_PATH);
    #endif

    while (running) {
        int fd = open(FIFO_PATH, O_RDONLY);
        if (fd < 0) {
            perror("Error opening server FIFO");
            exit(1);
        }

        ssize_t bytes = read(fd, buffer, sizeof(buffer));
        close(fd);

        if (bytes > 0) {
            buffer[bytes] = '\0';

            if (strncmp(buffer, "ADD|", 4) == 0) {
                handle_add(buffer);
            }
            else if (strncmp(buffer, "CONSULT|", 8) == 0) {
                handle_consult(buffer);
            }
            else if (strncmp(buffer, "DELETE|", 7) == 0) {
                handle_delete(buffer);
            }
            else if (strncmp(buffer, "LINES|", 6) == 0) {
                handle_lines(buffer);
            }
            else if (strncmp(buffer, "SEARCH|", 7) == 0) {
                handle_search(buffer);
            }
            else if (strncmp(buffer, "SEARCH_PAR|", 11) == 0) {
                handle_search_concurrent(buffer);
            }
            else if (strncmp(buffer, "SHUTDOWN|", 8) == 0) {
                handle_shutdown(buffer);
                read(fd, buffer, sizeof(buffer));
                running = 0;
                
            }
        }
    }

    unlink(FIFO_PATH);
    return 0;
}