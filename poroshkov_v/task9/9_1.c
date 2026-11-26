#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s файл\n", argv[0]);
        return 1;
    }

    pid_t child = fork();
    if (child == -1) {
        perror("fork");
        return 1;
    }

    if (child == 0) {
        execlp("cat", "cat", "--", argv[1], (char *)NULL);
        perror("cat");
        _exit(127);
    }

    else {
         printf("Родитель: PID=%ld, дочерний процесс=%ld\n", (long)getpid(), (long)child);
    fflush(stdout);
     puts("\nРодитель завершился");
     return 0;
    }

    int status;
    //while (waitpid(child, &status, 0) == -1) {
    //    if (errno != EINTR) {
    //        perror("waitpid");
    //        return 1;
    //    }
    //}

   
    //if (!WIFEXITED(status) || WEXITSTATUS(status) != 0 || fflush(stdout) == EOF) {
    //    return 1;
    //}

    return 0;
}

