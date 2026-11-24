#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <sys/sysmacros.h>

#if defined(__sun)
    #include <sys/acl.h>
#elif defined(__linux__)
    #include <acl/libacl.h>
    #include <sys/xattr.h>
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define SIX_MONTHS  (180L * 24 * 60 * 60)

void print_help(char *name) {
  printf("Usage: %s {path(s)}\n", name);
  printf("Description: shows information about directory/file.\n");
}

char get_file_type(mode_t mode) {
  if (S_ISREG(mode)) return '-';
  if (S_ISDIR(mode)) return 'd';
  if (S_ISLNK(mode)) return 'l';
  if (S_ISCHR(mode)) return 'c';
  if (S_ISBLK(mode)) return 'b';
  if (S_ISFIFO(mode)) return 'p';
  if (S_ISSOCK(mode)) return 's';
  return '\0';
}

int get_permissions(mode_t mode, char perms[]) {
  if (perms == NULL) {
    perror("perms array is not initialized");
    return 0;
  }

  perms[0] = get_file_type(mode);
  if (perms[0] == '\0') {
    perror("unknown data type");
    return 0;
  }

  perms[1] = (mode & S_IRUSR) ? 'r' : '-';
  perms[2] = (mode & S_IWUSR) ? 'w' : '-';
  if (mode & S_ISUID) {
    perms[3] = (mode & S_IXUSR) ? 's' : 'S';
  } else {
    perms[3] = (mode & S_IXUSR) ? 'x' : '-';
  }

  perms[4] = (mode & S_IRGRP) ? 'r' : '-';
  perms[5] = (mode & S_IWGRP) ? 'w' : '-';
  if (mode & S_ISGID) {
    perms[6] = (mode & S_IXGRP) ? 's' : 'S';
  } else {
    perms[6] = (mode & S_IXGRP) ? 'x' : '-';
  }

  perms[7] = (mode & S_IROTH) ? 'r' : '-';
  perms[8] = (mode & S_IWOTH) ? 'w' : '-';
  if (mode & S_ISVTX) {
    perms[9] = (mode & S_IXOTH) ? 't' : 'T';
  } else {
    perms[9] = (mode & S_IXOTH) ? 'x' : '-';
  }

  return 1;
}

int lsld(char *name, char *path, time_t now) {
  if (path == NULL) {
    return 0;
  }

  if (access(path, F_OK)) {
    printf("%s: cannot access '%s': No such file or directory\n", name, path);
    return 1;
  }

  struct stat st;
  char perms[12];

  if (lstat(path, &st)) {
    perror("lstat failed");
    return 0;
  }

  if (!get_permissions(st.st_mode, perms)) {
    perror("get_permissions failed");
    return 0;
  }

#if defined(__sun)
  int return_code = acl_trivial(path);
#elif defined(__linux__)
  int return_code = acl_extended_file(path);
#endif

  if (return_code == -1) {
    perror("acl_trivial failed");
    return 0;
  } else if (return_code == 1) {
    perms[10] = '+';
  } else {

  #if defined(__sun)
    return_code = pathconf(path, _PC_XATTR_EXISTS);
  #elif defined(__linux__)
    return_code = 0;
  #endif

    if (return_code == -1) {
      perror("pathconf failed");
      return 0;
    } else if (return_code == 1) {
      perms[10] = '@';
    } else {
      perms[10] = ' ';
    }
  }

  perms[11] = '\0';

  printf("%-10s %4lu ", perms, (unsigned long)st.st_nlink);

  struct passwd *pw = getpwuid(st.st_uid);
  struct group *gr = getgrgid(st.st_gid);

  if (pw == NULL) printf("%-8u ", (unsigned int)st.st_uid);
  else printf("%-8s ", pw->pw_name);

  if (gr == NULL) printf("%-8u ", (unsigned int)st.st_gid);
  else printf("%-8s ", gr->gr_name);

  char size_buf[32];
  if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode)) {
    snprintf(size_buf, sizeof(size_buf), "%u, %u",
             major(st.st_rdev), minor(st.st_rdev));
  } else {
    snprintf(size_buf, sizeof(size_buf), "%lld", (long long)st.st_size);
  }
  printf("%8s ", size_buf);

  char time_buf[64];
  struct tm *tm_info = localtime(&st.st_mtime);

  if (tm_info == NULL) {
    printf("%-12lld ", (long long)st.st_mtime);
  } else {
    if ((st.st_mtime < now - SIX_MONTHS) || (st.st_mtime > now)) {
      strftime(time_buf, sizeof(time_buf), "%b %e  %Y", tm_info);
    } else {
      strftime(time_buf, sizeof(time_buf), "%b %e %H:%M", tm_info);
    }
    printf("%-12s ", time_buf);
  }

  printf("%s", path);

  if (S_ISLNK(st.st_mode)) {
    char target[PATH_MAX];
    ssize_t len = readlink(path, target, sizeof(target) - 1);

    if (len != -1)  {
      target[len] = '\0';
      printf(" -> %s", target);
    } else {
      printf("%s: cannot read symbolic link %s", name, path);
    }
  }

  printf("\n");

  return 1;
}

int main(int argc, char *argv[]) {
  time_t now;
  time(&now);

  if (argc < 2) {
    if (!lsld(argv[0], ".", now)) {
      perror("lsld failed");
      return EXIT_FAILURE;
    }
  }

  for (int i = 1; i < argc; ++i) {
    if (!lsld(argv[0], argv[i], now)) {
      perror("lsld failed");
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
