#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <limits.h>

bool isBackground();
void backgroundCommand();
void foregroundCommand();
void splitCommand();
void scan();
void cd();
bool argumentsValidation();
void swapPointers(char** str1_ptr, char** str2_ptr);
char* getTilda();
void printPwd();

char* argv[10];
int argc;
char command[101];
char cwdCurr[PATH_MAX], cwdPrev[PATH_MAX];
char* currentPwd, * prevPwd;
char* homePwd;

int main() {
  currentPwd = getcwd(cwdCurr, sizeof(cwdCurr));
  prevPwd = getcwd(cwdPrev, sizeof(cwdPrev));
  homePwd = getTilda();

  scan();
  while (strcmp(command, "exit") != 0) {

    if (strlen(command) > 0) {

      // split the input
      splitCommand();
      if (!argumentsValidation()) {
        fprintf(stderr, "Error: Too many arguments\n");
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
  int status;

  //printf("Foreground\n"); //TODO: delete line before submit

  //forking, child process will execvp and parent will wait
  val = fork();
  if (val == 0) { ///child process
    printf("%d\n", (int) getpid());

    if (strcmp(argv[0], "cd") == 0) { // cd command
      cd();
    } else {
      status = execvp(argv[0], argv);
      if (status == -1) {
        fprintf(stderr, "Error in system call\n");
        exit(1);
      }
    }
  } else if (val > 0) { ///parent process
    wait(&status);
  }
}
void backgroundCommand() {
  pid_t val;
  int status;

  //printf("Background\n"); //TODO: delete line before submit

  //forking, child process will execvp, parent won't wait
  val = fork();
  if (val == 0) { ///child process
    printf("%d\n", (int) getpid());
    status = execvp(argv[0], argv);
    if (status == -1) {
      fprintf(stderr, "Error in system call\n");
      exit(1);
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
void cd() {
  int status;
  char* tempPwd = (char*) malloc((strlen(prevPwd) + 1) * sizeof(char));
  strcpy(tempPwd, prevPwd);
  prevPwd = getcwd(cwdPrev, sizeof(cwdPrev));

  if (argv[1] == NULL || strcmp(argv[1], "~") == 0) {
    status = chdir(homePwd);
    if (status == -1) {
      fprintf(stderr, "Error: No such file or directory\n");
      strcpy(prevPwd, tempPwd);
      free(tempPwd);
      exit(1);
    } else {
      currentPwd = getcwd(cwdCurr, sizeof(cwdCurr));
    }
  } else if (strcmp(argv[1], "-") == 0) {
    status = chdir(tempPwd);
    if (status == -1) {
      fprintf(stderr, "Error: No such file or directory\n");
      strcpy(prevPwd, tempPwd);
      free(tempPwd);
      exit(1);
    } else {
      currentPwd = getcwd(cwdCurr, sizeof(cwdCurr));
      //swapPointers(&currentPwd, &prevPwd);
    }
  } else {
    status = chdir(argv[1]);
    if (status == -1) {
      fprintf(stderr, "Error: No such file or directory\n");
      strcpy(prevPwd, tempPwd);
      free(tempPwd);
      exit(1);
    } else {
      currentPwd = getcwd(cwdCurr, sizeof(cwdCurr));
    }
  }
  printPwd();
  free(tempPwd);
}
void swapPointers(char** str1_ptr, char** str2_ptr) {
  char* temp = *str1_ptr;
  *str1_ptr = *str2_ptr;
  *str2_ptr = temp;
}
char* getTilda() {
  char cwd[PATH_MAX];
  char* tempPwd;

  tempPwd = getcwd(cwd, sizeof(cwd));
  char* tokenHome = strtok(tempPwd, "");
  char* tmp = strtok(tempPwd, "/home/");

  return tokenHome;
}
void printPwd() {
  printf("%s >> %s\n", prevPwd, currentPwd);
  //printf("prevPwd: %s\n",prevPwd);
  printf("\n");
}
