#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <string.h>

#define MAX_LINE 40

int main() {
  struct termios old_term;

  if (!isatty(STDIN_FILENO)) {
    perror("Standard input is not a terminal");
    return EXIT_FAILURE;
  }

  if (tcgetattr(STDIN_FILENO, &old_term) != 0) {
    perror("tcgetattr failed");
    return EXIT_FAILURE;
  }

  struct termios new_term = old_term;
  new_term.c_lflag &= ~ICANON & ~ECHO;

  if (tcsetattr(STDIN_FILENO, TCSANOW, &new_term) != 0) {
    perror("tcsetattr failed");
    return EXIT_FAILURE;
  }

  char erasec = old_term.c_cc[VERASE];
  char killc = old_term.c_cc[VKILL];
  char werasec = old_term.c_cc[VWERASE];
  char eofc = old_term.c_cc[VEOF];

  size_t len = 0, word = 0;
  char line[MAX_LINE];
  setbuf(stdout, NULL);
  while (1) {
    int ch = getchar();

    if (ch == EOF) break;
    if (ch == '\r') ch = '\n';
    if (ch == '\x1b') {
      ch = getchar();

      if(ch == '[' || ch == 'O') {
        do { ch = getchar(); }
        while (ch != EOF && ('a' < ch || ch > 'z') && ('A' < ch || ch > 'Z') && ch != '~');
      }

      putchar('\a');
    }

    if (ch == erasec) {
      if (len) {
        fputs("\b \b", stdout);
        len--;
        if (word) word--;
      }
    } else if (ch == killc) {
      fputs("\r\033[K", stdout);
      len = 0;
      word = 0;
    } else if (ch == werasec) {
      size_t new_len = len;

      while (new_len > 0 && isspace((unsigned char)line[new_len - 1])) new_len--;
      while (new_len > 0 && !isspace((unsigned char)line[new_len - 1])) new_len--;

      while (len > new_len) {
        fputs("\b \b", stdout);
        len--;
      }

      word = 0;
    } else if (ch == eofc) {
      if (!len) {
        break;
      }
    } else if (ch == '\n') {
      putchar(ch);
      len = 0;
      word = 0;
    } else if (!isprint((unsigned char)ch)) {
      putchar('\a');
    } else if (len == MAX_LINE && word < MAX_LINE) {
      if (isspace((unsigned char)ch)) {
        putchar('\n');
        len = 0;
        word = 0;
      } else if (!word) {
        putchar('\n');
        putchar(ch);
        len = 1;
        word = 1;
        line[0] = (char)ch;
      } else {

        for (size_t i = 0; i < word; ++i) {
          fputs("\b \b", stdout);
        }

        putchar('\n');

        size_t word_temp = word;
        char line_temp[MAX_LINE];

        while (word) {
          putchar(line[len - word]);
          line_temp[word_temp - word] = line[len - word];
          word--;
        }

        putchar(ch);
        line_temp[word + 1] = (char)ch;

        strncpy(line, line_temp, MAX_LINE);

        word = word_temp + 1;
        len = word;
      }
    } else if (word >= MAX_LINE) {
      putchar('\n');
      putchar(ch);

      len = 0;
      line[len++] = (char)ch;

      if (!isspace((unsigned char)ch)) {
        word = 1;
      } else {
        word = 0;
      }
    } else {
      line[len++] = (char)ch;
      putchar(ch);

      if (!isspace((unsigned char)ch)) {
        word++;
      } else {
        word = 0;
      }
    }
  }

  if (tcsetattr(STDIN_FILENO, TCSANOW, &old_term) != 0) {
    perror("tcsetattr failed");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
