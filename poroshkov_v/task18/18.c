#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <inttypes.h>

static int print_entry(char *path) {
    struct stat st;
    if (lstat(path, &st) == -1) {
        perror(path);
        return 1;
    }

    char mode[] = "----------";
    if (S_ISDIR(st.st_mode)) {
        mode[0] = 'd';
    } else if (!S_ISREG(st.st_mode)) {
        mode[0] = '?';
    }

    mode_t bits[] = {S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP,
                     S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};
    for (int i = 0; i < 9; ++i) {
        if (st.st_mode & bits[i]) {
            mode[i + 1] = "rwxrwxrwx"[i];
        }
    }

    struct passwd *owner = getpwuid(st.st_uid);

    char user[256];
    char group[256];
    char size[32] = "";
    char date[32];
    if (owner) {
        snprintf(user, sizeof(user), "%s", owner->pw_name);
    } else {
        snprintf(user, sizeof(user), "%ju", (uintmax_t)st.st_uid);
    }

    struct group *gr = getgrgid(st.st_gid);
    if (gr) {
        snprintf(group, sizeof(group), "%s", gr->gr_name);
    } else {
        snprintf(group, sizeof(group), "%ju", (uintmax_t)st.st_gid);
    }

    if (S_ISREG(st.st_mode)) {
        snprintf(size, sizeof(size), "%jd", (intmax_t)st.st_size);
    }

    struct tm *tm = localtime(&st.st_mtime);
    if (!tm || !strftime(date, sizeof(date), "%Y-%m-%d %H:%M", tm)) {
        return 1;
    }

    size_t length = strlen(path);
    while (length > 1 && path[length - 1] == '/') {
        path[--length] = '\0';
    }

    char *name = strrchr(path, '/');
    name = name && name[1] ? name + 1 : path;
    return printf("%s %5ju %-16s %-16s %12s %s %s\n", mode, (uintmax_t)st.st_nlink, user, group,
                  size, date, name) < 0;
}

int main(int argc, char *argv[]) {
    int status = 0;

    char current[] = ".";
    if (argc == 1) {
        status = print_entry(current);
    }

    for (int i = 1; i < argc; ++i) {
        if (print_entry(argv[i])) {
            status = 1;
        }
    }

    if (fflush(stdout) == EOF) {
        status = 1;
    }

    return status;
}
