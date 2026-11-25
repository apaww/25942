#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <ctype.h>

int main() {
  int fd[2];
  pipe(fd);

  pid_t pid = fork();

  if (pid < 0) {
    perror("fork failed");
    return EXIT_FAILURE;
  } else if (pid == 0) {

    close(fd[0]);

    char *text = "I'm at a bootloader Tryna choose, hold up Which Linux distro should I boot into? Half are too painful Half are just shameful Don't even think about Gentoo Though non-proprietary software does exist I can't last one minute without a driver fix Was windows really that much worse than this? Should I rm -rf and call it quits?";
    write(fd[1], text, strlen(text));

    close(fd[1]);
    wait(NULL);

  } else {

    close(fd[1]);

    char buffer[4096];
    ssize_t bytes_read;

    while ((bytes_read = read(fd[0], buffer, sizeof(buffer))) > 0) {
      for (int i = 0; i < bytes_read; ++i) {
        buffer[i] = toupper(buffer[i]);
      }
      write(STDOUT_FILENO, buffer, bytes_read);
    }

    return EXIT_SUCCESS;
  }

  return EXIT_SUCCESS;
}
