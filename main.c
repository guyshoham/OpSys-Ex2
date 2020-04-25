#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>

bool isBackground();
void backgroundCommand();
void foregroundCommand();
void splitCommand();

char* argv[10];
int argc;
char command[100];

int main() {

  ///scan command input
  printf("> ");
  gets(command);
  while (strcmp(command, "exit")) {

    if (strlen(command) > 0) {

      // split the input
      splitCommand();

      //make the command (foreground or background)
      if (isBackground()) {
        // if background, remove '&' from argv
        argv[argc - 1] = NULL;
        backgroundCommand(argv);
      } else {
        foregroundCommand(argv);
      }

    }// end of empty string check

    ///scan another command input
    printf("> ");
    gets(command);
  }

  return 0;
}
void splitCommand() {
  argc = 0;
  char* token = strtok(command, " ");

  while (token != NULL) {
    argv[argc] = token;
    argc++;
    token = strtok(NULL, " ");
  }
  argv[argc] = NULL;
}
void foregroundCommand() {
  pid_t val;
  int stat;
  int status;

  printf("Foreground\n"); //TODO: delete line before submit

  //forking, child process will execvp and parent will wait
  val = fork();
  if (val == 0) { ///child process
    printf("%d\n", (int) getpid());
    status = execvp(argv[0], argv);
    if (status == -1) {
      printf("exec failed\n");
    }
  } else if (val > 0) { ///parent process
    wait(&stat);
  }
}
void backgroundCommand() {
  pid_t val;
  int status;

  printf("Background\n"); //TODO: delete line before submit

  //forking, child process will execvp, parent won't wait
  val = fork();
  if (val == 0) { ///child process
    printf("%d\n", (int) getpid());
    status = execvp(argv[0], argv);
    if (status == -1) {
      printf("exec failed\n");
    }
  }
}
bool isBackground() {
  return strcmp(argv[argc - 1], "&") == 0 ? true : false;
}

