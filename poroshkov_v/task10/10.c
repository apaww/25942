#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s команда [аргументы...]\n", argv[0]);
        return 1;
    }

    pid_t child = fork();
    if (child == -1) {
        perror("fork");
        return 1;
    }

    if (child == 0) {
        execvp(argv[1], argv + 1);
        int error = errno;
        perror(argv[1]);
        _exit(error == ENOENT ? 127 : 126);
    }

    int status;
    while (waitpid(child, &status, 0) == -1) {
        if (errno != EINTR) {
            perror("waitpid");
            return 1;
        }
    }

    if (WIFEXITED(status)) {
        int code = WEXITSTATUS(status);
        printf("Код завершения: %d\n", code);
        return fflush(stdout) == EOF ? 1 : code;
    }

    int sig = WTERMSIG(status);
    printf("Процесс завершён сигналом %d\n", sig);
    return fflush(stdout) == EOF ? 1 : 128 + sig;
}
