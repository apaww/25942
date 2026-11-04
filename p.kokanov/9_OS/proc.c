#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>

#define BUFFER_SIZE 4096

int main () {
  pid_t pid;

  pid = fork();

  if (pid < 0) {
    perror("fork failed");
    return EXIT_FAILURE;
  } else if (pid == 0) {
    printf("\n---- CHILD WAS BORN!\n");

    FILE *fp = popen("cat file", "r");
    
    if (fp == NULL) {
        perror("popen failed");
        return EXIT_FAILURE;
    }

    char buffer[BUFFER_SIZE];
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
    }

    pclose(fp);

    printf("---- CHILD IS ABOUT TO BE KILLED\n\n");

    exit(EXIT_SUCCESS);
  } else {
    printf("I dont pay child support\n");
    
    int status; // siginfo_t info;
    waitpid(pid, &status, 0); // wait(&status) or waitid(P_PID, pid, &info, WEXITED);
    
    printf("Cause i dont have kids\n");
  }

  return EXIT_SUCCESS;
}
