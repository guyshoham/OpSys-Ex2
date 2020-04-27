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
char* getTilda();
void printPwd();

char command[101], cwdCurr[PATH_MAX], cwdPrev[PATH_MAX];
char* argv[10], * currentPwd, * prevPwd, * homePwd;
int argc;
pid_t* children[100];
int childrenCount = 0;
char** commands[100];

int main() {
  currentPwd = getcwd(cwdCurr, sizeof(cwdCurr));
  prevPwd = getcwd(cwdPrev, sizeof(cwdPrev));
  homePwd = getTilda();

  scan();
  while (strcmp(command, "exit") != 0) {

    if (strlen(command) > 0) {

      //save command to history, before the split (save with all flags)
      char* cmdPtr;
      cmdPtr = (char*) malloc((strlen(command) + 1) * sizeof(char));
      strcpy(cmdPtr, command);
      commands[childrenCount] = cmdPtr;

      // split the input
      splitCommand();
      if (!argumentsValidation()) {
        fprintf(stderr, "Error: Too many arguments\n");
      } else {
        //make the command (foreground or background)
        if (isBackground()) {
          // if background, remove '&' from argv
          argv[argc - 1] = NULL;
          backgroundCommand();
        } else {
          foregroundCommand();
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

  //forking, child process will execvp and parent will wait
  val = fork();
  if (val == 0) { ///child process
    if (!strcmp(argv[0], "cd")) { // cd command
      printf("%d\n", (int) getpid());
      cd();
    } else if (!strcmp(argv[0], "jobs")) {
      for (int i = 0; i < childrenCount; ++i) {
        if (kill(children[i], 0) == 0) {
          printf("%d %s\n", children[i], commands[i]);
        }
      }
      _exit(1);
    } else if (!strcmp(argv[0], "history")) {
      children[childrenCount] = getpid();
      childrenCount++;
      for (int i = 0; i < childrenCount; ++i) {
        printf("%d %s ", children[i], commands[i]);
        if (kill(children[i], 0) == 0) {
          printf("RUNNING\n");
        } else {
          printf("DONE\n");
        }
      }
      _exit(1);
    } else {
      printf("%d\n", (int) getpid());
      status = execvp(argv[0], argv);
      if (status == -1) {
        fprintf(stderr, "Error in system call\n");
        _exit(1);
      }
    }
  } else if (val > 0) { ///parent process
    wait(&status);
    children[childrenCount] = val;
    childrenCount++;
  }
}
void backgroundCommand() {
  pid_t val;
  int status;

  //forking, child process will execvp, parent won't wait
  val = fork();
  if (val == 0) { ///child process
    printf("%d\n", (int) getpid());
    status = execvp(argv[0], argv);
    if (status == -1) {
      fprintf(stderr, "Error in system call\n");
      _exit(1);
    }
  } else if (val > 0) { ///parent process
    children[childrenCount] = val;
    childrenCount++;
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
      _exit(1);
    } else {
      currentPwd = getcwd(cwdCurr, sizeof(cwdCurr));
    }
  } else if (strcmp(argv[1], "-") == 0) {
    status = chdir(tempPwd);
    if (status == -1) {
      fprintf(stderr, "Error: No such file or directory\n");
      strcpy(prevPwd, tempPwd);
      free(tempPwd);
      _exit(1);
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
      _exit(1);
    } else {
      currentPwd = getcwd(cwdCurr, sizeof(cwdCurr));
    }
  }
  printPwd();
  free(tempPwd);
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
