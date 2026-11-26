#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

int main(int argc, char *argv[])
{
    struct stat st;
    int error = 0;

    int count = (argc > 1) ? argc - 1 : 1;

    for (int i = 0; i < count; ++i)
    {
        const char *path = (argc > 1) ? argv[i + 1] : ".";

        if (lstat(path, &st) == -1)
        {
            perror(path);
            error = 1;
            continue;
        }

        // type
        char type;

        if (S_ISDIR(st.st_mode)) type = 'd';
        else if (S_ISREG(st.st_mode)) type = '-';
        else type = '?';

        // permissions
        char permissions[10];

        mode_t bits[9] = {
            S_IRUSR, S_IWUSR, S_IXUSR,
            S_IRGRP, S_IWGRP, S_IXGRP,
            S_IROTH, S_IWOTH, S_IXOTH
        };

        char letters[] = "rwxrwxrwx";

        for (int j = 0; j < 9; ++j)
        {
            permissions[j] = (st.st_mode & bits[j]) ? letters[j] : '-';
        }

        permissions[9] = '\0';

        // owner, group
        struct passwd *pw = getpwuid(st.st_uid);
        struct group *gr = getgrgid(st.st_gid);

        char owner[128];
        char group[128];

        if (pw != NULL)
            snprintf(owner, sizeof(owner), "%s", pw->pw_name);
        else
            snprintf(owner, sizeof(owner), "%lu", (unsigned long)st.st_uid);

        if (gr != NULL)
            snprintf(group, sizeof(group), "%s", gr->gr_name);
        else
            snprintf(group, sizeof(group), "%lu", (unsigned long)st.st_gid);

        // size
        char size[32] = "";

        if (S_ISREG(st.st_mode))
        {
            snprintf(size, sizeof(size), "%lld", (long long)st.st_size);
        }

        // time
        char date[32] = "";
        struct tm *tm_info = localtime(&st.st_mtime);

        if (tm_info != NULL)
        {
            strftime(date, sizeof(date), "%Y-%m-%d %H:%M", tm_info);
        }

        // name
        size_t end = strlen(path);

        while (end > 1 && path[end - 1] == '/')
            --end;

        size_t start = end;

        while (start > 0 && path[start - 1] != '/')
            --start;

        const char *name = path + start;
        int name_length = (int)(end - start);

        if (name_length == 0)
        {
            name = "/";
            name_length = 1;
        }

        printf("%c%s %3lu %-12s %-12s %10s %16s %.*s\n",
               type, permissions, (unsigned long)st.st_nlink, owner, group, size, date, name_length, name);
    }

    return error;
}

