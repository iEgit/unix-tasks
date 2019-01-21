#include <stdio.h>
#include <stdlib.h>
#define BUFFER_SIZE 256

void pipeStreams(FILE* inFile, FILE* outFile) {
  char buffer[BUFFER_SIZE];
  int bytesRead;
  bzero(buffer, BUFFER_SIZE);

  while ((bytesRead = fread(buffer, sizeof(char), BUFFER_SIZE - 2, inFile))) {
    if (bytesRead < 0) {
        perror("ERROR reading from file");
        exit(1);
    }

    fprintf(outFile, "%s", buffer);
    bzero(buffer, BUFFER_SIZE);
  }
}
