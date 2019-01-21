// https://www.tutorialspoint.com/unix_sockets/socket_client_example.htm
#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

#include "common.c"
#define BUFFER_SIZE 256

int connectToHost(char* hostName, int port) {
  struct sockaddr_in serverAddress;
  struct hostent *server;

  /* Create a socket point */
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    perror("ERROR opening socket");
    exit(1);
  }

  server = gethostbyname(hostName);

  if (server == NULL) {
    fprintf(stderr,"ERROR, no such host\n");
    exit(0);
  }

  bzero((char *) &serverAddress, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serverAddress.sin_addr.s_addr, server->h_length);
  serverAddress.sin_port = htons(port);

  /* Now connect to the server */
  if (connect(sockfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
    perror("ERROR connecting");
    exit(1);
  }

  return sockfd;
}

int main(int argc, char *argv[]) {
  int sockfd, port, n;

  char buffer[BUFFER_SIZE];

  if (argc < 3) {
    fprintf(stderr,"usage %s hostname port\n", argv[0]);
    exit(1);
  }

  port = atoi(argv[2]);

  int bytesRead = 0;
  bzero(buffer, BUFFER_SIZE);
  printf("$: ");

  while ((fgets(buffer, BUFFER_SIZE, stdin)) != NULL) {

    if (strcmp(buffer, "quit\n") == 0) {
      exit(0);
    }

    sockfd = connectToHost(argv[1], port);
    FILE* sockFile = fdopen(sockfd, "r+");

    fprintf(sockFile, "%s", buffer);

    for (int i = 0; i < BUFFER_SIZE; i++) {
      if (buffer[i] == '\n') {
        buffer[i] = '\0';
      }
    }


    if (strcmp(buffer, "ls") == 0) {
      pipeStreams(sockFile, stdout);
    } else if (strncmp(buffer, "get", 3) == 0) {
      FILE* outFile = fopen(buffer + 4, "wb");
      pipeStreams(sockFile, outFile);
      fclose(outFile);
    } else if (strncmp(buffer, "put", 3) == 0) {
      FILE* inFile = fopen(buffer + 4, "r");
      pipeStreams(inFile, sockFile);
      fclose(inFile);
    }

    fclose(sockFile);
    close(sockfd);
    printf("\n$: ");
  }

  return 0;
}
