
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

int main(void)
{
    sigset_t signals;
    struct sigaction sa;
    unsigned long long count = 0;
    int sig;

    // Signals to process
    sigemptyset(&signals);
    sigaddset(&signals, SIGINT);
    sigaddset(&signals, SIGQUIT);

    // Block signals for sigwait
    if (sigprocmask(SIG_BLOCK, &signals, NULL) == -1)
    {
        perror("sigprocmask");
        return 1;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGTSTP, &sa, NULL) == -1)
    {
        perror("sigaction");
        return 1;
    }

    printf("Ctrl+C - sound\n");
    printf("Ctrl+\\ - show count and exit\n");
    fflush(stdout);

    while (1)
    {
        sig = sigwait(&signals);

        if (sig == -1)
        {
            perror("sigwait");
            return 1;
        }

        if (sig == SIGINT)
        {
            ++count;
            printf("\a Sound, count: %llu\n", count);
            fflush(stdout);
        }
        else if (sig == SIGQUIT)
        {
            printf("\nTotal sounds: %llu\n", count);
            break;
        }
    }

    return 0;
}

