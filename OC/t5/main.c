#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define INITIAL_CAPACITY 64

typedef struct {
    off_t offset;
    int length;
} LineInfo;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Использование\n", argv[0]);
        return 1;
    }

    
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Ошибка открытия файла");
        return 1;
    }

    int capacity = INITIAL_CAPACITY;
    int line_count = 0;
    LineInfo *table = malloc(capacity * sizeof(LineInfo));
    if (!table) {
        perror("Ошибка выделения памяти под таблицу");
        close(fd);
        return 1;
    }

    char ch; 
    int current_len = 0; 
    table[0].offset = 0; 

    while (read(fd, &ch, 1) == 1) {
        if (ch == '\n') {
            
            table[line_count].length = current_len;
            line_count++;
            current_len = 0;

            
            if (line_count >= capacity) {
                capacity *= 2;
                LineInfo *temp = realloc(table, capacity * sizeof(LineInfo));
                if (!temp) {
                    perror("Ошибка расширения памяти таблицы");
                    free(table);
                    close(fd);
                    return 1;
                }
                table = temp;
            }

            
            table[line_count].offset = lseek(fd, 0L, SEEK_CUR);
        } else {
           
            current_len++;
        }
    }

    
    if (current_len > 0) {
        table[line_count].length = current_len;
        line_count++;
    }

    printf("--- Таблица строк (Debug) ---\n");
    for (int i = 0; i < line_count; i++) {
        printf("Строка %d: Смещение = %ld, Длина = %d\n", 
               i + 1, (long)table[i].offset, table[i].length);
    }
    printf("Всего найдено строк: %d\n", line_count);
    printf("-----------------------------\n\n");

    int line_num;
    while (1) {
        printf("Введите номер строки (0 для выхода): ");
        if (scanf("%d", &line_num) != 1) {
            
            printf("Некорректный ввод!\n");
            break;
        }

        
        if (line_num == 0) {
            break;
        }

        
        if (line_num < 1 || line_num > line_count) {
            printf("Строки с номером %d не существует!\n", line_num);
            continue;
        }

        
        char *line_buf = malloc(table[line_num - 1].length + 1);
        if (!line_buf) {
            perror("Ошибка выделения буфера под строку");
            continue;
        }

       
        lseek(fd, table[line_num - 1].offset, SEEK_SET);

        
        read(fd, line_buf, table[line_num - 1].length);

        
        line_buf[table[line_num - 1].length] = '\0';

        printf("%s\n", line_buf);

        free(line_buf);
    }

    free(table);
    close(fd);

    return 0;
}