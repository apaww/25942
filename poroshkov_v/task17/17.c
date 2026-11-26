#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>

#define WIDTH 40

enum {
    KEY_CTRL_D = 4,
    KEY_BACKSPACE = '\b',
    KEY_CTRL_W = 23,
    KEY_ESCAPE = 27,
    KEY_DELETE = 127
};

static struct termios saved;

static void restore_terminal(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &saved);
}

static void stop_handler(int sig) {
    restore_terminal();
    _exit(128 + sig);
}

static void redraw(const wchar_t line[], int length) {
    putwchar(L'\r');
    for (int i = 0; i < WIDTH; ++i) {
        putwchar(L' ');
    }
    putwchar(L'\r');
    for (int i = 0; i < length; ++i) {
        putwchar(line[i]);
    }
    fflush(stdout);
}

static wint_t read_key(void) {
    wint_t c = getwchar();
    while (c == KEY_ESCAPE) {
        putwchar(L'\a');
        fflush(stdout);

        c = getwchar();
        if (c == L'[' || c == L'O') {
            do {
                c = getwchar();
            } while (c >= L' ' && c < KEY_DELETE && (c < L'@' || c == L'['));

            if (c >= L'@' && c <= L'~') {
                c = getwchar();
            }
        }
    }
    return c;
}

int main(void) {
    if (setlocale(LC_CTYPE, "") == NULL) {
        fputs("Не удалось установить локаль\n", stderr);
        return 1;
    }
    if (tcgetattr(STDIN_FILENO, &saved) == -1) {
        perror("tcgetattr");
        return 1;
    }
    if (atexit(restore_terminal) != 0) {
        return 1;
    }

    struct sigaction sa = {0};
    sa.sa_handler = stop_handler;
    sigemptyset(&sa.sa_mask);
    int signals[] = {SIGINT, SIGQUIT, SIGTERM, SIGHUP, SIGPIPE};
    for (size_t i = 0; i < sizeof(signals) / sizeof(signals[0]); ++i) {
        if (sigaction(signals[i], &sa, NULL) == -1) {
            perror("sigaction");
            return 1;
        }
    }

    struct termios mode = saved;
    mode.c_lflag &= ~(ICANON | ECHO | ECHONL | IEXTEN | ISIG);
    mode.c_iflag &= ~(IXON | IXOFF);
    mode.c_cc[VMIN] = 1;
    mode.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &mode) == -1) {
        perror("tcsetattr");
        return 1;
    }

    wchar_t line[WIDTH];
    int length = 0;
    wint_t c;
    while ((c = read_key()) != WEOF) {
        if (c == KEY_CTRL_D) {
            if (length == 0) {
                break;
            }
            putwchar(L'\a');
        } else if (c == saved.c_cc[VERASE] || c == KEY_BACKSPACE || c == KEY_DELETE) {
            if (length > 0) {
                --length;
                redraw(line, length);
            }
        } else if (c == saved.c_cc[VKILL]) {
            length = 0;
            redraw(line, length);
        } else if (c == KEY_CTRL_W) {
            while (length > 0 && iswspace(line[length - 1])) {
                --length;
            }
            while (length > 0 && !iswspace(line[length - 1])) {
                --length;
            }
            redraw(line, length);
        } else if (c == L'\n' || c == L'\r') {
            putwchar(L'\n');
            length = 0;
        } else if (!iswprint(c)) {
            putwchar(L'\a');
        } else {
            if (length == WIDTH || wcswidth(line, length) + wcwidth(c) > WIDTH) {
                if (iswspace(c)) {
                    putwchar(L'\n');
                    length = 0;
                    fflush(stdout);
                    continue;
                }

                int start = length;
                while (start > 0 && !iswspace(line[start - 1])) {
                    --start;
                }
                if (start == 0) {
                    start = length;
                }
                redraw(line, start);
                putwchar(L'\n');
                length -= start;
                wmemmove(line, line + start, length);
                redraw(line, length);
            }
            line[length] = (wchar_t)c;
            ++length;
            putwchar(c);
        }
        fflush(stdout);
    }

    putwchar(L'\n');
    if (ferror(stdin)) {
        perror("Ввод");
        return 1;
    }
    return fflush(stdout) == EOF || ferror(stdout);
}
