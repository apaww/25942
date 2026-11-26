#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ulimit.h>
#include <sys/resource.h>

extern char **environ;

int main(int argc, char *argv[]) {
    int opt;
    char cwd[1024];
    struct rlimit rl;

    while ((opt = getopt(argc, argv, "ispruU:cC:dvV:")) != -1) {
        switch (opt) {
            case 'i': /* Реальные и эффективные UID и GID */
                printf("UID: real=%ld, eff=%ld | GID: real=%ld, eff=%ld\n",
                       (long)getuid(), (long)geteuid(), (long)getgid(), (long)getegid());
                break;

            case 's': /* Сделать процесс лидером группы */
                if (setpgid(0, 0) == -1)
                    perror("setpgid");
                else
                    printf("New group leader, PGID: %ld\n", (long)getpgrp());
                break;

            case 'p': /* PID, PPID и PGID */
                printf("PID: %ld, PPID: %ld, PGID: %ld\n",
                       (long)getpid(), (long)getppid(), (long)getpgrp());
                break;

            case 'u': /* Значение ulimit */
                printf("ulimit: %ld\n", ulimit(UL_GETFSIZE, 0));
                break;

            case 'U': /* Изменить ulimit */
                if (ulimit(UL_SETFSIZE, atol(optarg)) == -1)
                    perror("ulimit set");
                else
                    printf("ulimit changed to %s\n", optarg);
                break;

            case 'c': /* Размер core-файла */
                getrlimit(RLIMIT_CORE, &rl);
                printf("Core file size: cur=%lu, max=%lu\n",
                       (unsigned long)rl.rlim_cur, (unsigned long)rl.rlim_max);
                break;

            case 'C': /* Изменить размер core-файла */
                getrlimit(RLIMIT_CORE, &rl);
                rl.rlim_cur = (rlim_t)atol(optarg);
                if (setrlimit(RLIMIT_CORE, &rl) == -1)
                    perror("setrlimit");
                else
                    printf("Core limit changed to %s\n", optarg);
                break;

            case 'd': /* Текущая директория */
                if (getcwd(cwd, sizeof(cwd)) != NULL)
                    printf("CWD: %s\n", cwd);
                else
                    perror("getcwd");
                break;

            case 'v': /* Переменные окружения */
                for (char **env = environ; *env != NULL; ++env)
                    printf("%s\n", *env);
                break;

            case 'V': /* Установить переменную окружения (name=value) */
                if (putenv(optarg) != 0)
                    perror("putenv");
                else
                    printf("Set env: %s\n", optarg);
                break;

            case '?': /* Неизвестная опция */
                fprintf(stderr, "Unknown option: -%c\n", optopt);
                break;
        }
    }

    return 0;
}
