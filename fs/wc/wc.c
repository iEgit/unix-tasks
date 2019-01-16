#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <cwctype>
#include <wchar.h>
#include <locale.h>


#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

wint_t NEW_LINE = L'\n';

enum STATES {DEFAULT, SPACE};
long long int totalLines = 0;
long long int totalWords = 0;
long long int totalChars = 0;

void wc(FILE* file) {
  long long int lines = 0;
  long long int words = 0;
  long long int chars = 0;

  wchar_t currentChar = 0;

  enum STATES state = DEFAULT;

  while ( (currentChar = fgetwc(file)) != WEOF ) {
    chars++;

    if (iswspace(currentChar)) {
      words += state != SPACE;
      lines += currentChar == NEW_LINE;
      state = SPACE;
    } else {
      state = DEFAULT;
    }
  }

  words += state != SPACE;

  printf("%lld %lld %lld", lines, words, chars);
  totalLines += lines;
  totalChars += chars;
  totalWords += words;
}

int main(int argc, char** argv) {
  setlocale(LC_ALL, "C.UTF-8");
  setlocale(LC_CTYPE, "C.UTF-8");

  if (argc == 1) {
    wc(stdin);
    printf("\n");
  } else {
    for (int i = 1; i < argc; i++) {
      FILE* file = fopen(argv[i], "r");
      wc(file);
      printf(" %s\n", argv[i]);
      fclose(file);
    }

    if (argc > 2) {
      printf("total %lld %lld %lld\n", totalLines, totalWords, totalChars);
    }
  }
  return 0;

}
