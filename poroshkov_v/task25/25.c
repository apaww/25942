#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc > 2) {
        fprintf(stderr, "Использование: %s [текст]\n", argv[0]);
        return 1;
    }

    int channel[2];
    if (pipe(channel) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t child = fork();
    if (child == -1) {
        perror("fork");
        close(channel[0]);
        close(channel[1]);
        return 1;
    }

    if (child == 0) {
        close(channel[1]);
        FILE *input = fdopen(channel[0], "r");
        if (!input) {
            perror("fdopen");
            _exit(1);
        }
        int c;
        while ((c = fgetc(input)) != EOF) {
            if (putchar(toupper((unsigned char)c)) == EOF) {
                break;
            }
        }
        int failed = ferror(input) || ferror(stdout);
        fclose(input);
        if (fflush(stdout) == EOF) {
            failed = 1;
        }
        _exit(failed);
    }

    close(channel[0]);
    signal(SIGPIPE, SIG_IGN);

    FILE *output = fdopen(channel[1], "w");

    int failed = output == NULL;
    if (output) {
        if (fprintf(output, "%s\n", argc == 2 ? argv[1] : "Hello from Parent!") < 0) {
            failed = 1;
        }
        if (fclose(output) == EOF) {
            failed = 1;
        }
    } else {
        perror("fdopen");
        close(channel[1]);
    }

    int status;
    while (waitpid(child, &status, 0) == -1) {
        if (errno != EINTR) {
            perror("waitpid");
            return 1;
        }
    }

    if (failed || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        return 1;
    }

    return 0;
}

