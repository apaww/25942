#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    int fd[2];
    pid_t pid;
    char buf[256];
    ssize_t n;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <text>\n", argv[0]);
        return 1;
    }

    if (pipe(fd) == -1)
    {
        perror("pipe");
        return 1;
    }

    pid = fork();

    if (pid == -1)
    {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        return 1;
    }

    if (pid == 0)
    {
        // Child
        close(fd[0]);

        const char *text = argv[1];
        size_t length = strlen(text);

        while (length > 0)
        {
            ssize_t written = write(fd[1], text, length);

            if (written == -1)
            {
                if (errno == EINTR)
                    continue;

                perror("write");
                close(fd[1]);
                _exit(1);
            }

            text += written;
            length -= written;
        }

        close(fd[1]);
        _exit(0);
    }
    else
    {
        // Parent
        close(fd[1]);

        while (1)
        {
            n = read(fd[0], buf, sizeof(buf));

            if (n == 0)
                break;

            if (n == -1)
            {
                if (errno == EINTR)
                    continue;

                perror("read");
                close(fd[0]);
                waitpid(pid, NULL, 0);
                return 1;
            }

            for (ssize_t i = 0; i < n; ++i)
            {
                putchar(toupper((unsigned char)buf[i]));
            }
        }

        printf("\n");

        close(fd[0]);

        if (waitpid(pid, NULL, 0) == -1)
        {
            perror("waitpid");
            return 1;
        }
    }

    return 0;
}

