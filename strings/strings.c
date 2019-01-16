#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#define min(a,b) (((a) < (b)) ? (a) : (b))

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>


void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

enum STATES {DEFAULT, PIPE, AND, QUOTE, DOUBLE_QUOTE};

size_t BUF_SIZE = 10;
char* BUFFER;
FILE* tmpFile;

int wordsLastIndex = 0;

typedef struct Word {
  int start;
  int end;
} Word;

Word** WORDS;
int WORDS_LEN;

size_t COMPARATOR_BUF_SIZE = 1000;
char* LEFT_WORD_BUFFER;
char* RIGHT_WORD_BUFFER;

int wordComparator(Word* a, Word* b) {
  if (a == b) {
    return 0;
  }

  int aLength = a->end - a->start;
  int bLength = b->end - b->start;

  int bytesRead = 0;
  fseek(tmpFile, a->start, SEEK_SET);
  bytesRead = fread(LEFT_WORD_BUFFER, sizeof(char), min(aLength, COMPARATOR_BUF_SIZE - 1), tmpFile);
  /* printf("asked %d, bytes read %d, left word %s, right word %s", */
  /*     min(aLength, COMPARATOR_BUF_SIZE - 1), */
  /*     bytesRead, */
  /*     LEFT_WORD_BUFFER, */
  /*     RIGHT_WORD_BUFFER); */
  LEFT_WORD_BUFFER[bytesRead] = '\0';

  fseek(tmpFile, b->start, SEEK_SET);
  bytesRead = fread(RIGHT_WORD_BUFFER, sizeof(char), min(bLength, COMPARATOR_BUF_SIZE - 1), tmpFile);
  RIGHT_WORD_BUFFER[bytesRead] = '\0';

  /* printf("comparing words %s %s\n", LEFT_WORD_BUFFER, RIGHT_WORD_BUFFER); */

  int result = strcmp(LEFT_WORD_BUFFER, RIGHT_WORD_BUFFER);
  /* printf("a start %d, a end %d, a length %d  ", a->start, a->end, aLength); */
  /* printf("b start %d, b end %d, b length %d\n", b->start, b->end, bLength); */
  /* printf("left: %s$ | right: %s$ | result %d\n", LEFT_WORD_BUFFER, RIGHT_WORD_BUFFER, result); */

  // if we have too big word on the right or on the left
  // and the result is still 0
  // compare next part of the words
  if (result == 0 && (aLength > COMPARATOR_BUF_SIZE - 1 || bLength > COMPARATOR_BUF_SIZE - 1)) {
    Word* tmpA = (Word*) malloc(sizeof(Word));
    Word* tmpB = (Word*) malloc(sizeof(Word));

    tmpA->start = min(a->start + COMPARATOR_BUF_SIZE - 1, a->end);
    tmpA->end = a->end;

    tmpB->start = min(b->start + COMPARATOR_BUF_SIZE - 1, b->end);
    tmpB->end = b->end;

    result = wordComparator(tmpA, tmpB);
    free(tmpA);
    free(tmpB);
    return result;
  }


  return result;

}

int comparator(const void* a, const void* b) {

  /* printf("a start %d, a end %d\n", (*(Word**)a) ->start, 0); */
  // a is a WORD** pointer to a pointer
  // dereference it once to obtain a pointer to Word
  return wordComparator( *((Word**) a), *((Word**) b));
}

void pushWord(Word* word) {
  if (wordsLastIndex >= WORDS_LEN) {
    int oldSize = WORDS_LEN * sizeof(Word*);

    WORDS_LEN *= 2;
    int newSize = WORDS_LEN * sizeof(Word*);
    Word** newWords = malloc(newSize);

    memcpy(newWords, WORDS, oldSize);
    WORDS = newWords;
  }

  /* printf("pushed a start %d, a end %d\n", word->start, word->end); */
  WORDS[wordsLastIndex++] = word;
}
bool isSeparator(char symbol) {
  return isspace(symbol);
}

void digestBuffer(int bytesRead) {
  static int offset = 0;
  static bool lastWordCompleted = true;
  /* printf("bytes read %d, completed: %d, offset: %d\n", bytesRead, (int)lastWordCompleted, offset); */
  int bufferIndex = 0;
  Word* word;

  while(bufferIndex < bytesRead) {
    // skip separators
    while (isspace(BUFFER[bufferIndex])) {
      // if new buffer starts with separator then the last word has ended exactly in offset - 1
      if (!lastWordCompleted) {
        lastWordCompleted = true;
        WORDS[wordsLastIndex - 1]->end = offset;
        /* printf("ended with start %d and end %d\n", */
        /*     WORDS[wordsLastIndex - 1]->start, */
        /*     WORDS[wordsLastIndex - 1]->end */
        /*     ); */
      }

      if (bufferIndex >= bytesRead) {
        break;
      }

      bufferIndex++;
    }

    if (bufferIndex >= bytesRead) {
      break;
    }

    // here we have start of the word
    // or continuation of previous
    if (lastWordCompleted) {
      word = malloc(sizeof(Word));
      word->start = offset + bufferIndex;
      word->end = word->start;
      pushWord(word);
    } else {
      lastWordCompleted = true;
      word = WORDS[wordsLastIndex - 1];
    }


    // skip non separators
    while (!isSeparator(BUFFER[bufferIndex])) {
      if (bufferIndex >= bytesRead) {
        // we are at the end of buffer and the word hasn't ended yet
        // it will be continued in the next buffer
        lastWordCompleted = false;
        break;
      }

      bufferIndex++;
    }

    if (lastWordCompleted) {
      // here we have end of the word
      word->end = offset + bufferIndex;
        /* printf("ended with start %d and end %d\n", */
        /*     word->start, */
        /*     word->end); */
      /* printf("pushed a start %d, a end %d\n", word->start, word->end); */
    }
  }


  for (int i = 0; i < wordsLastIndex; i++) {
    /* printf("start: %d, end: %d\n", WORDS[i]->start, WORDS[i]->end); */
    /* printf("word: %.*s\n", END_INDEXES[i] - START_INDEXES[i] + 1 , BUFFER + START_INDEXES[i]); */
  }

  for (int i = 0; i < wordsLastIndex; i++) {
    /* printf("%d\n", INDEXES[i]); */
  }
  /* qsort(INDEXES, wordsLastIndex, sizeof(int), comparator); */

  for (int i = 0; i < wordsLastIndex; i++) {
    /* printf("%d\n", INDEXES[i]); */
  }

  /* return wordsLastIndex; */
  offset += bytesRead;
}

void writeToFile(int bytesRead) {
  fwrite(BUFFER, sizeof(char), bytesRead, tmpFile);
  fflush(tmpFile);
}

void sortAndPrint() {
  /* printf("---------------------------------------------------------------\n"); */
  qsort(WORDS, wordsLastIndex, sizeof(Word*), comparator);
  /* printf("here --------------------------------------- -----------"); */
  for (size_t i = 0; i < wordsLastIndex; i++) {

    /* printf("i %d, start %d, end %d \n",i, WORDS[i]->start, WORDS[i]->end); */
    fseek(tmpFile, WORDS[i]->start, SEEK_SET);

    /* printf("i: %d,start: %d, end:%d",i, WORDS[i]->start, WORDS[i]->end); */
    for (int j = WORDS[i]->start; j < WORDS[i]->end; j++) {
      printf("%c", fgetc(tmpFile));
    }
    printf("\n");
  }
}

int main(int argc, char** argv) {
  signal(SIGSEGV, handler);

  BUFFER = malloc(sizeof(char) * BUF_SIZE);
  LEFT_WORD_BUFFER = malloc(sizeof(char) * COMPARATOR_BUF_SIZE);
  RIGHT_WORD_BUFFER = malloc(sizeof(char) * COMPARATOR_BUF_SIZE);
  WORDS_LEN = 1;
  WORDS = malloc(WORDS_LEN * sizeof(Word*));

  tmpFile = fopen("tmp", "w+");

  int bytesRead = 0;
  int iteration = 0;

  char currentChar = 0;
  char prevChar = 0;
  int bytesToDigest = 0;

  enum STATES state = DEFAULT;

  while( (bytesRead = read(0, &currentChar, 1)) ) {
    switch (currentChar) {

      case '&':
        if (state == QUOTE || state == DOUBLE_QUOTE) {
          break;
        }

        if (state == AND) {
          state = DEFAULT;
        } else {
          BUFFER[bytesToDigest] = ' ';
          bytesToDigest++;
          state = AND;
        }
        break;

      case '|':
        if (state == QUOTE || state == DOUBLE_QUOTE) {
          break;
        }

        if (state == PIPE) {
          state = DEFAULT;
        } else {
          BUFFER[bytesToDigest] = ' ';
          bytesToDigest++;
          state = PIPE;
        }
        break;

      case ';':
        if (state == QUOTE || state == DOUBLE_QUOTE) {
          break;
        }

        BUFFER[bytesToDigest] = ' ';
        bytesToDigest++;
        BUFFER[bytesToDigest] = ';';
        bytesToDigest++;
        currentChar = ' ';
        break;

      case '\'':
        if (state == DOUBLE_QUOTE) {
          break;
        }
        // single separator followed by a '
        if (state == AND || state == PIPE) {
          BUFFER[bytesToDigest] = ' ';
          bytesToDigest++;
        }

        if (state == QUOTE) {
          state = DEFAULT;
        } else {
          state = QUOTE;
        }

        currentChar = ' ';

        break;

      case '"':
        if (state == QUOTE) {
          break;
        }

        // single separator followed by a "
        if (state == AND || state == PIPE) {
          BUFFER[bytesToDigest] = ' ';
          bytesToDigest++;
        }

        if (state == DOUBLE_QUOTE) {
          state = DEFAULT;
        } else {
          state = DOUBLE_QUOTE;
        }

        currentChar = ' ';

        break;

      default:
        // single separator followed by a character
        if (state == AND || state == PIPE) {
          BUFFER[bytesToDigest] = ' ';
          bytesToDigest++;
        }
        state = DEFAULT;
        break;
    }

    BUFFER[bytesToDigest] = currentChar;
    bytesToDigest++;

    if (bytesToDigest >= BUF_SIZE - 2) {
      digestBuffer(bytesToDigest);
      writeToFile(bytesToDigest);

      bytesToDigest = 0;
    }
  }

  if (bytesToDigest) {
    digestBuffer(bytesToDigest);
    writeToFile(bytesToDigest);
  }

  BUFFER[0] = ' ';
  digestBuffer(1);

  sortAndPrint();
  fclose(tmpFile);

  return 0;
}
