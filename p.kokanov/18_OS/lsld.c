#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <sys/acl.h>

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

int lsld(char *name, char *path) {
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

  int return_code = acl_trivial(path);

  if (return_code == -1) {
    perror("acl_trivial failed");
    return 0;
  } else if (return_code == 1) {
    perms[10] = '+';
  } else {
    return_code = pathconf(path, _PC_XATTR_EXISTS);
    
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

  printf("%s %lu\n", perms, (unsigned long)st.st_nlink);
  
  return 1;
}

int main(int argc, char *argv[]) {
  for (int i = 1; i < argc; ++i) {
    if (!lsld(argv[0], argv[i])) {
      perror("lsld failed");
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
