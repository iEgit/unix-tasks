#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <cwctype>
#include <wchar.h>
#include <locale.h>

#include <string.h>
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

#include <dirent.h>
#define SEPARATOR "/"

int stackSize = 10;
char** stack;
int top = -1;

bool isempty() {
  return top == -1;
}

char* pop() {
   if(!isempty()) {
     return stack[top--];
   }
}

void push(char* data) {
  top++;

  if (top >= stackSize) {
    int oldSize = stackSize * sizeof(char*);

    stackSize *= 2;
    int newSize = stackSize * sizeof(char*);
    char** newStack = (char**)malloc(newSize);

    memcpy(newStack, stack, oldSize);
    stack = newStack;
  }

  stack[top] = data;
}

void printPermissions(mode_t fileMode) {
    printf( (S_ISDIR(fileMode)) ? "d" : "-");
    printf( (fileMode & S_IRUSR) ? "r" : "-");
    printf( (fileMode & S_IWUSR) ? "w" : "-");
    printf( (fileMode & S_IXUSR) ? "x" : "-");
    printf( (fileMode & S_IRGRP) ? "r" : "-");
    printf( (fileMode & S_IWGRP) ? "w" : "-");
    printf( (fileMode & S_IXGRP) ? "x" : "-");
    printf( (fileMode & S_IROTH) ? "r" : "-");
    printf( (fileMode & S_IWOTH) ? "w" : "-");
    printf( (fileMode & S_IXOTH) ? "x" : "-");
}

char* getUserName(uid_t uid) {
  struct passwd* passwdEntry = getpwuid(uid);
  if (passwdEntry == NULL) {
    return "";
  }

  return passwdEntry->pw_name;
}

char* getGroupName(gid_t gid) {
  struct group* groupEntry = getgrgid(gid);
  if (groupEntry == NULL) {
    return "";
  }
  return groupEntry->gr_name;
}

void ls(char *path) {
    DIR *directory = opendir(path);

    if (directory != NULL) {
        struct dirent* entry;

        while ((entry = readdir(directory)) != NULL) {

            if (entry->d_name == NULL) {
              continue;
            }


            // skip .. and  .
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char* pathBuffer = (char*)malloc(strlen(path) + strlen(entry->d_name) + 2);
            strcat(strcat(strcpy(pathBuffer, path), SEPARATOR), entry->d_name);

            struct stat statBuffer;

            if (stat(pathBuffer, &statBuffer) == 0) {

                char timeBuffer[26];
                struct tm* tm_info;
                const time_t* mtime = &statBuffer.st_mtime;

                tm_info = localtime(mtime);

                strftime(timeBuffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

                printPermissions(statBuffer.st_mode);
                printf(" %s %s %d %s %s\n",
                    getUserName(statBuffer.st_uid),
                    getGroupName(statBuffer.st_gid),
                    statBuffer.st_size,
                    timeBuffer,
                    pathBuffer);

                if (S_ISDIR(statBuffer.st_mode)) {
                    push(pathBuffer);
                }
            }
        }

        closedir(directory);
    }
}

int main(int argc, char** argv) {
  stack = (char**)malloc(sizeof(char*) * stackSize);
  char* directory = (char*)malloc(26 * sizeof(char));

  if (argc == 2) {
    strcpy(directory, argv[1]);
  } else {
    strcpy(directory, ".");
  }

  push(directory);

  while (!isempty()) {

    char* dir = pop();
    if (argc > 1) {
      printf("\n%s:\n", dir);
    }

    ls(dir);

    free(dir);
  }

  return 0;
}
