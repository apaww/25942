#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>

int main() {
  struct termios old_term;

  if (!isatty(STDIN_FILENO)) {
    perror("STDIN_FILENO is not tty");
    return EXIT_FAILURE;
  }

  if (tcgetattr(STDIN_FILENO, &old_term) == -1) {
    perror("tcgetattr failed");
    return EXIT_FAILURE;
  }

  struct termios new_term = old_term;
  new_term.c_lflag &= ~(ISIG | ICANON);
  char vintr = new_term.c_cc[VINTR];
  char vquit = new_term.c_cc[VQUIT];
  new_term.c_cc[VMIN] = 1;
  new_term.c_cc[VTIME] = 0;

  if (tcsetattr(STDIN_FILENO, TCSANOW, &new_term) == -1) {
    perror("tcsetattr failed");
    return EXIT_FAILURE;
  }

  int beeps = 0;
  while (1) {
    int ch = getchar();

    if (ch == vintr) {
      putchar('\a');
      fflush(stdout);
      beeps++;
    } else if (ch == vquit) break;
  }

  if (tcsetattr(STDIN_FILENO, TCSANOW, &old_term) == -1) {
    perror("tcsetattr failed");
    return EXIT_FAILURE;
  }

  printf("\nbeeped %d times :d\n", beeps);

  return EXIT_SUCCESS;
}
