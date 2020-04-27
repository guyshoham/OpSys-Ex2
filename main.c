#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <limits.h>
#include <pwd.h>

bool isBackground();
void backgroundCommand();
void foregroundCommand();
void splitCommand();
void scan();
void cd();
bool argumentsValidation();
char* getTilda();
char* replaceAll(const char* s, const char* oldStr, const char* newStr);

char command[101], cwdCurr[PATH_MAX], cwdPrev[PATH_MAX];
char* argv[10], * currentPwd, * prevPwd, * homePwd;
int argc;
pid_t* children[100];
int childrenCount = 0;
char** commands[100];
bool isChild = false;

int main() {
  currentPwd = getcwd(cwdCurr, sizeof(cwdCurr));
  prevPwd = getcwd(cwdPrev, sizeof(cwdPrev));
  homePwd = getTilda();

  scan();
  while (strcmp(command, "exit")) {

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

      // terminate child processes that reach here after executing command
      if (isChild) { return 0; }

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
  if (val == 0) { //child process
    isChild = true;
    if (!strcmp(argv[0], "cd")) { // cd command
      printf("%d\n", (int) getpid());
      cd();
    } else if (!strcmp(argv[0], "jobs")) {
      int i;
      for (i = 0; i < childrenCount; ++i) {
        if (kill(children[i], 0) == 0) {
          printf("%d %s\n", children[i], commands[i]);
        }
      }
    } else if (!strcmp(argv[0], "history")) {
      children[childrenCount] = getpid();
      childrenCount++;
      int i;
      for (i = 0; i < childrenCount; ++i) {
        printf("%d %s ", children[i], commands[i]);
        if (kill(children[i], 0) == 0) {
          printf("RUNNING\n");
        } else {
          printf("DONE\n");
        }
      }
    } else {
      printf("%d\n", (int) getpid());
      status = execvp(argv[0], argv);
      if (status == -1) {
        fprintf(stderr, "Error in system call\n");
      }
    }
  } else if (val > 0) { //parent process
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
  if (val == 0) { //child process
    isChild = true;
    printf("%d\n", (int) getpid());
    status = execvp(argv[0], argv);
    if (status == -1) {
      fprintf(stderr, "Error in system call\n");
    }
  } else if (val > 0) { //parent process
    children[childrenCount] = val;
    childrenCount++;
  }
}
bool argumentsValidation() {

  int argCounter = 0, i;

  for (i = 1; argv[i] != NULL; ++i) {
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
    } else {
      currentPwd = getcwd(cwdCurr, sizeof(cwdCurr));
    }
  } else if (strcmp(argv[1], "-") == 0) {
    status = chdir(tempPwd);
    if (status == -1) {
      fprintf(stderr, "Error: No such file or directory\n");
      strcpy(prevPwd, tempPwd);
      free(tempPwd);
    } else {
      currentPwd = getcwd(cwdCurr, sizeof(cwdCurr));
      //swapPointers(&currentPwd, &prevPwd);
    }
  } else {
    argv[1] = replaceAll(argv[1], "~", homePwd);
    status = chdir(argv[1]);
    if (status == -1) {
      fprintf(stderr, "Error: No such file or directory\n");
      strcpy(prevPwd, tempPwd);
      free(tempPwd);
    } else {
      currentPwd = getcwd(cwdCurr, sizeof(cwdCurr));
    }
    free(argv[1]);
  }
  free(tempPwd);
}
char* getTilda() { return getpwuid(getuid())->pw_dir; }
char* replaceAll(const char* s, const char* oldStr, const char* newStr) {
  char* result;
  int i, count = 0;
  int oldStrlen = strlen(oldStr);
  int newStrlen = strlen(newStr);

  // Counting the number of times old word occur in the string
  for (i = 0; s[i] != '\0'; i++) {
    if (strstr(&s[i], oldStr) == &s[i]) {
      count++;

      // Jumping to index after the old word.
      i += oldStrlen - 1;
    }
  }

  // Making new string of enough length
  result = (char*) malloc(i + count * (newStrlen - oldStrlen) + 1);

  i = 0;
  while (*s) {
    // compare the substring with the result
    if (strstr(s, oldStr) == s) {
      strcpy(&result[i], newStr);
      i += newStrlen;
      s += oldStrlen;
    } else
      result[i++] = *s++;
  }

  result[i] = '\0';
  return result;
}
