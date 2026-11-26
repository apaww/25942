#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>

#define MAX_LINES 1024
#define MAX LINE_LEN 1024

int main(int argc, char *argv[]) {
    const char *filename = (argc > 1) ? argv[1] : "text.txt";

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    off_t offsets[MAX_LINES];
    size_t lenghts[MAX_LINES];
    int line_count = 0;

    char ch;
}
