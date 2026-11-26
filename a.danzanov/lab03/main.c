#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void check_access(const char *filename) {
    printf("Real UID: %ld, Effective UID: %ld\n", (long)getuid(), (long)getuid());

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("fopen");
    } else {
        printf("File '%s' opened successfully\n", filename);
        fclose(fp);
    }
}

int main(int argc, char *argv[]) {
    const char *filename = (argc > 1) ? argv[1] : "file";

    check_access(filename);

    if(setuid(getuid()) == -1) {
        perror("setuid");
    }

    check_access(filename);

    return 0;
}
