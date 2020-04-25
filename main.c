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
void scan();
bool argumentsValidation();

char* argv[10];
int argc;
char command[100];

int main() {

  scan();
  while (strcmp(command, "exit") != 0) {

    if (strlen(command) > 0) {

      // split the input
      splitCommand();
      if (!argumentsValidation()) {
        printf("Error: Too many arguments\n");
      } else {
        //make the command (foreground or background)
        if (isBackground()) {
          // if background, remove '&' from argv
          argv[argc - 1] = NULL;
          backgroundCommand(argv);
        } else {
          foregroundCommand(argv);
        }
      }

    }// end of empty string check

    scan();
  }

  return 0;
}
void scan() {
  ///scan command input
  printf("> ");
  gets(command);
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
bool isBackground() {
  return strcmp(argv[argc - 1], "&") == 0 ? true : false;
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
bool argumentsValidation() {

  int argCounter = 0;

  for (int i = 1; argv[i] != NULL; ++i) {
    //printf("%c\n", argv[i][0]);
    if (argv[i][0] != '-' && argv[i][0] != '&') {
      argCounter++;
    }
  }

  return argCounter > 1 ? false : true;
}
