#include "guiao1.h"

int main(int argc, char const *argv[]) {
    int res;

    for (int i = 0; i<argc; i++){
        printf("->arg[%d] = %s\n", i, argv[i]);
    }

    if (argc == 1) {
        res = mycat();
    } else if (argc == 2) {
        res = mycat_adv(argv[1]);
    } else if (argc == 4) {
        res = mycp(argv[1], argv[2], argv[3]);
    } else {
        perror("Wrong args!");
        res = 1;
    }

    return res;
}