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
void jobs();
void history();
bool argumentsValidation();
char* getTilda();
char* replaceAll(const char* s, const char* oldStr, const char* newStr);

char command[101], cwdCurr[PATH_MAX], cwdPrev[PATH_MAX];
char* argv[10], * currentPwd, * prevPwd, * homePwd;
int argc;
pid_t pids[100];
pid_t parentPid;
int pidCount = 0;
char** commands[100];
bool isChild = false;

int main() {
  parentPid = getpid();
  currentPwd = getcwd(cwdCurr, sizeof(cwdCurr));
  prevPwd = getcwd(cwdPrev, sizeof(cwdPrev));
  homePwd = getTilda();

  scan();
  while (true) {

    if (strlen(command) > 0) {
      //save command to history, before the split (save with all flags)
      char* cmdPtr;
      cmdPtr = (char*) malloc((strlen(command) + 1) * sizeof(char));
      strcpy(cmdPtr, command);
      commands[pidCount] = cmdPtr;

      // split the input
      splitCommand();
      //check exit command
      if (!strcmp(argv[0], "exit")) { // cd command
        printf("%d\n", parentPid);
        exit(0);
      }

      //check cd command
      if (!strcmp(argv[0], "cd")) { // cd command
        printf("%d\n", parentPid);
        cd();
        pids[pidCount] = parentPid;
        pidCount++;
      }
        //check foreground or background command
      else if (isBackground()) {
        // if background, remove '&' from argv
        argv[argc - 1] = NULL;
        backgroundCommand();
      } else {
        foregroundCommand();
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
  bool isEcho = false, hasQuotes = false;
  int quotesPos = 0;
  char* token = strtok(command, " ");

  while (token != NULL) {
    argv[argc] = token;

    //check if command is echo
    if (argc == 0) {
      if (strcmp(argv[0], "echo") == 0) {
        isEcho = true;
      }
    }

    //if echo, check if first arg has opening quotes
    if (isEcho && argc == 1) {
      if (argv[1][0] == '\"') {
        hasQuotes = true;
        quotesPos = argc;
        //remove opening quotes
        argv[1]++;
      }
    }

    argc++;
    token = strtok(NULL, " ");
  } //end of while token != NULL

  // if we removed the opening quotes, we shall find the ending quotes and remove them as well
  if (hasQuotes) {
    //remove ending quotes
    int w = 0;
    while (hasQuotes) {
      if (argv[quotesPos + w][strlen(argv[quotesPos + w]) - 1] == '\"') {
        argv[quotesPos + w][strlen(argv[quotesPos + w]) - 1] = 0;
        hasQuotes = false;
      }
      w++;
    }// end of while
  } //end of if hasQuotes

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
    if (!strcmp(argv[0], "jobs")) { jobs(); }
    else if (!strcmp(argv[0], "history")) { history(); }
    else {
      printf("%d\n", getpid());
      status = execvp(argv[0], argv);
      if (status == -1) {
        fprintf(stderr, "Error in system call\n");
      }
    }
  } else if (val > 0) { //parent process
    wait(&status);
    pids[pidCount] = val;
    pidCount++;
  }
}
void backgroundCommand() {
  pid_t val;
  int status;

  //forking, child process will execvp, parent won't wait
  val = fork();
  if (val == 0) { //child process
    isChild = true;
    printf("%d\n", getpid());
    status = execvp(argv[0], argv);
    if (status == -1) {
      fprintf(stderr, "Error in system call\n");
    }
  } else if (val > 0) { //parent process
    pids[pidCount] = val;
    pidCount++;
  }
}
bool argumentsValidation() {

  int argCounter = 0, i;

  for (i = 1; argv[i] != NULL; ++i) {
    if (argv[i][0] != '-' && argv[i][0] != '&') {
      argCounter++;
    }
  }

  return argCounter > 1 ? false : true;
}
void cd() {

  if (!argumentsValidation()) {
    fprintf(stderr, "Error: Too many arguments\n");
    return;
  }

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
void jobs() {
  int i;
  for (i = 0; i < pidCount; ++i) {
    if (kill(pids[i], 0) == 0 && pids[i] != parentPid) {
      printf("%d %s\n", pids[i], commands[i]);
    }
  }
}
void history() {
  pids[pidCount] = getpid();
  pidCount++;
  int i;
  for (i = 0; i < pidCount; ++i) {
    printf("%d %s ", pids[i], commands[i]);
    if (kill(pids[i], 0) != 0 || pids[i] == parentPid) {
      printf("DONE\n");
    } else {
      printf("RUNNING\n");
    }
  }
}
