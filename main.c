#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
  int status;
  char command[100];
  char* argv[10];
  pid_t val;
  int stat;

  ///scan command input
  printf("> ");
  gets(command);
  while (strcmp(command, "exit")) {

    // split the input
    char* token = strtok(command, " ");
    int argc = 0;

    while (token != NULL) {
      argv[argc] = token;
      argc++;
      printf("%s\n", token);
      token = strtok(NULL, " ");
    }
    argv[argc] = NULL;

    //make the command (foreground or background)
    if (strcmp(argv[argc - 1], "&") == 0) {
      // if background, remove '&' from argv
      argv[argc - 1] = NULL;
      printf("Background\n");

      //forking, child process will execvp, parent won't wait
      val = fork();
      if (val == 0) { ///child process
        printf("%d\n", (int) getpid());
        status = execvp(argv[0], argv);
        if (status == -1) {
          printf("exec failed\n");
        }
        return 0;
      }
    } else {
      printf("Foreground\n");

      //forking, child process will execvp and parent will wait
      val = fork();
      if (val == 0) { ///child process
        printf("%d\n", (int) getpid());
        status = execvp(argv[0], argv);
        if (status == -1) {
          printf("exec failed\n");
        }
        return 0;
      } else if (val > 0) { ///parent process
        wait(&stat);
      }
    }

    ///scan another command input
    printf("> ");
    gets(command);
  }

  return 0;
}
