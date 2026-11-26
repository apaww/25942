#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void check_file_access(const char *path)
{
   
    FILE *target_stream = fopen(path, "r");
    
    if (target_stream == NULL) {
        perror("error");
    } else {
        printf("%s", path);
        fclose(target_stream);
    }
}


void display_process_identity(void)
{
    uid_t ruid = getuid();
    uid_t euid = geteuid();
    printf("RUID: %d | EUID: %d\n", (int)ruid, (int)euid);
}

int main(int argument_count, char *argument_values[])
{
    
    if (argument_count != 2) {
        fprintf(stderr, "Критическая ошибка\n", argument_values[0]);
        exit(EXIT_FAILURE);
    }

    const char *target_file = argument_values[1];

    
    display_process_identity();
    check_file_access(target_file);

   
    if (setuid(getuid()) < 0) {
        perror("");
        return EXIT_FAILURE;
    }

    
    display_process_identity();
    check_file_access(target_file);

    return EXIT_SUCCESS;
}
