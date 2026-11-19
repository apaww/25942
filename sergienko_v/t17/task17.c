#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <errno.h>

#define WIDTH 40
#define MAX_INPUT 4096

static struct termios old_term;
static int rows = 1;

void restore_terminal(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
}

void new_row(int *col, int *r)
{
    printf("\r\n");
    *col = 0;
    ++(*r);
}

void redraw(const char *line, int len)
{
    printf("\r");
    if (rows > 1) printf("\033[%dA", rows - 1);
    printf("\033[J");

    int col = 0;
    int r = 1;

    for (int i = 0; i < len;)
    {
        if (line[i] == ' ')
        {
            if (col == WIDTH) new_row(&col, &r);
            putchar(' ');
            ++col;
            ++i;
            continue;
        }

        int end = i;
        while (end < len && line[end] != ' ') ++end;

        if (col > 0 && col + end - i > WIDTH)
            new_row(&col, &r);

        while (i < end)
        {
            if (col == WIDTH) new_row(&col, &r);
            putchar(line[i++]);
            ++col;
        }
    }

    rows = r;
    fflush(stdout);
}

int main(void)
{
    if (!isatty(STDIN_FILENO))
    {
        fprintf(stderr, "Run in a terminal\n");
        return 1;
    }

    if (tcgetattr(STDIN_FILENO, &old_term) == -1)
    {
        perror("tcgetattr");
        return 1;
    }

    struct termios new_term = old_term;
    new_term.c_lflag &= ~(ICANON | ECHO | ISIG | IEXTEN);
    new_term.c_iflag &= ~IXON;
    new_term.c_cc[VMIN] = 1;
    new_term.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_term) == -1)
    {
        perror("tcsetattr");
        return 1;
    }

    atexit(restore_terminal);

    char line[MAX_INPUT + 1];
    int len = 0;
    int escape = 0;
    unsigned char c;

    printf("Line editor (40 columns)\r\n");
    printf("Backspace: erase, Ctrl+U: kill, Ctrl+W: word\r\n");
    printf("Ctrl+D on empty line: exit\r\n");
    fflush(stdout);

    while (1)
    {
        ssize_t n = read(STDIN_FILENO, &c, 1);

        if (n == -1)
        {
            if (errno == EINTR) continue;
            perror("read");
            return 1;
        }
        if (n == 0) break;

        // Ignore escape sequences (arrows, function keys)
        if (escape == 1)
        {
            if (c == '[' || c == 'O') escape = 2;
            else escape = 0;
            continue;
        }

        if (escape == 2)
        {
            if (c >= 0x40 && c <= 0x7E) escape = 0;
            continue;
        }

        if (c == 27)
        {
            putchar('\a');
            fflush(stdout);
            escape = 1;
            continue;
        }

        if (c == old_term.c_cc[VERASE] || c == 127 || c == 8)
        {
            if (len > 0) --len;
            else putchar('\a');
        }
        else if (c == old_term.c_cc[VKILL])
        {
            len = 0;
        }
        else if (c == 23) // Ctrl+W
        {
            while (len > 0 && line[len - 1] == ' ') --len;
            while (len > 0 && line[len - 1] != ' ') --len;
        }
        else if (c == 4) // Ctrl+D
        {
            if (len == 0)
            {
                printf("\r\n");
                break;
            }
            putchar('\a');
        }
        else if (c == '\n' || c == '\r')
        {
            printf("\r\n");
            len = 0;
            rows = 1;
            continue;
        }
        else if (isprint(c))
        {
            if (len < MAX_INPUT) line[len++] = c;
            else putchar('\a');
        }
        else
        {
            putchar('\a');
        }

        redraw(line, len);
    }

    return 0;
}

