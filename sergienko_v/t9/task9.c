#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    pid_t pid;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    pid = fork();

    if (pid == -1)
    {
        perror("fork");
        return 1;
    }

    if (pid == 0)
    {
        // Child process
        execlp("cat", "cat", argv[1], (char *)NULL);

        perror("execlp");
        _exit(1);
    }
    else
    {
        // Parent process
        printf("Parent: child process started\n");
        fflush(stdout);

        if (waitpid(pid, NULL, 0) == -1)
        {
            perror("waitpid");
            return 1;
        }

        printf("Parent: child process finished\n");
    }

    return 0;
}

