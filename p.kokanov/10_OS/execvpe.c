#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

extern char **environ;

int alloc_default_path(char **path) {
  size_t size = confstr(_CS_PATH, NULL, 0);

  if (size == 0) {
    perror("confstr failed");
    return -1;
  }

  char *buffer = malloc(size);
  if (buffer == NULL) {
    perror("malloc failed");
    return -1;
  }

  if (confstr(_CS_PATH, buffer, size) == 0) {
    perror("confstr failed");
    free(buffer);
    return -1;
  }

  *path = buffer;
  return 0;
}

int execvpe(const char *file, char *const argv[], char *const envp[]) {
  if (strchr(file, '/')) {
    return execve(file, argv, envp);
  }

  char *path = getenv("PATH");

  int allocated = 0;
  if (!path) {
    if (alloc_default_path(&path) == -1) {
      return -1;
    }
    allocated = 1;
  }

  char *p = path;
  while (1) {
    char *end = strchr(p, ':');
    size_t len = end ? (size_t)(end - p) : strlen(p);

    char *buffer = malloc(len + 1 + strlen(file) + 1);
    if (!buffer) {
      perror("malloc failed");
      if (allocated) free(path);
      return -1;
    }

    if (len == 0) {
      strcpy(buffer, file);
    } else {
      memcpy(buffer, p, len);
      buffer[len] = '/';
      strcpy(buffer + len + 1, file);
    }

    execve(buffer, argv, envp);
    free(buffer);

    if (!end) break;
    p = end + 1;
  }

  if (allocated) free(path);
  return -1;
}

int main() {
  pid_t pid = fork();

  if (pid < 0) {
    perror("fork failed");
    return EXIT_FAILURE;
  } else if (pid == 0) {
    char *file = "getopt";
    char *argv[] = {file, "-v", NULL};
    char *envp[] = {NULL};

    printf("execvpe will run %s from PATH with NULL envp right now\n\n", file);
    execvpe(file, argv, envp);

    perror("execvpe failed");
    return EXIT_FAILURE;
  } else {
    printf("\n----------\nexecvpe test 1\n----------\n");
    wait(NULL);
    printf("\n----------\ndone!\n");
  }

  pid = fork();

  if (pid < 0) {
    perror("fork failed");
    return EXIT_FAILURE;
  } else if (pid == 0) {
    char *file = "getopt";
    char *argv[] = {file, "-v", NULL};

    printf("execvpe will run %s from PATH with current envp right now\n\n", file);
    execvpe(file, argv, environ);

    perror("execvpe failed");
    return EXIT_FAILURE;
  } else {
    printf("\n----------\nexecvpe test 2\n----------\n");
    wait(NULL);
    printf("\n----------\ndone!\n");
  }

  return EXIT_SUCCESS;
}

