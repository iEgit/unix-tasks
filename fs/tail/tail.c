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

wint_t NEW_LINE = L'\n';

enum STATES {DEFAULT, SPACE};

typedef struct Line {
  int id;
  Line* next;
  Line* prev;
  FILE* fileStream;
  char* fileName;
} Line;

int numberOfLines = 0;

Line* createLine(Line* prev, Line* next) {
  numberOfLines++;

  char* name;

  Line* line = (Line*)malloc(sizeof(Line));
  line->id = numberOfLines;
  line->prev = prev;
  line->next = next;


  // init file
  asprintf(&name, ".tail.tmp.%d", line->id);
  int memSize = strlen(name) * sizeof(char);
  line->fileName = (char*)malloc(memSize);
  line->fileName = (char*)memcpy(line->fileName, name, memSize);

  free(name);
  line->fileStream = fopen(line->fileName, "w+");

  return line;
}

void resetFileStream(Line* line) {
  freopen(NULL, "w+", line->fileStream);
}

void printList(Line* line) {
  wint_t currentChar = 0;

  while (line) {
    freopen(NULL, "rw+", line->fileStream);

    while ( (currentChar = fgetwc(line->fileStream)) != WEOF ) {
      printf("%lc", currentChar);
    }

    line = line->next;
  }
}

void freeList(Line* line) {
  Line* tmp;
  while (line) {
    tmp = line;
    fclose(line->fileStream);
    unlink(line->fileName);


    line = line->next;
    free(tmp);
  }
}

void tail(FILE* file) {
  numberOfLines = 0;

  Line* head = createLine(NULL, NULL);
  Line* tail = head;

  int linesNeeded = 10;

  wint_t currentChar = 0;
  bool isNewLine = false;
  while ( (currentChar = fgetwc(file)) != WEOF ) {
    if (isNewLine) {

      /* printf("number:%d\n", numberOfLines); */
      if (numberOfLines < linesNeeded) {
        /* printf("adding new line\n"); */
        Line* newTail = createLine(tail, NULL);
        tail->next = newTail;
        tail = newTail;
      } else {
        /* printf("changing line\n"); */
        Line* newHead = head->next;
        newHead->prev = NULL;

        tail->next = head;
        head->prev = tail;
        head->next = NULL;

        tail = head;
        head = newHead;
      }

        resetFileStream(tail);
    }

    isNewLine = currentChar == NEW_LINE;
    fprintf(tail->fileStream, "%lc", currentChar);
  }

  printList(head);
  freeList(head);
}

int main(int argc, char** argv) {
  setlocale(LC_ALL, "C.UTF-8");
  setlocale(LC_CTYPE, "C.UTF-8");

  if (argc == 1) {
    tail(stdin);
  } else {
    for (int i = 1; i < argc; i++) {
      FILE* file = fopen(argv[i], "r");
      if (argc > 2) {
        printf("%s\n", argv[i]);
      }
      tail(file);
      fclose(file);
    }
  }

  return 0;
}
