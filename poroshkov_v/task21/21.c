#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <signal.h>

int main(void) {
    sigset_t signals;

    sigemptyset(&signals);
    sigaddset(&signals, SIGINT);
    sigaddset(&signals, SIGQUIT);

    if (sigprocmask(SIG_BLOCK, &signals, NULL) == -1) {
        perror("sigprocmask");
        return 1;
    }

    int count = 0;

    puts("Ctrl+C — звуковой сигнал, Ctrl+\\ — завершение");
    fflush(stdout);

    for (;;) {
        int signum;

        if (sigwait(&signals, &signum) != 0) {
            return 1;
        }

        if (signum == SIGINT) {
            putchar('\a');
            fflush(stdout);
            count++;
        } else if (signum == SIGQUIT) {
            printf("\nСигнал прозвучал %d раз\n", count);
            break;
        }
    }

    return 0;
}
