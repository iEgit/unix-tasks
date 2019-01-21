// https://www.tutorialspoint.com/unix_sockets/socket_server_example.htm

#define SEPARATOR "/"
#include <netdb.h>
#include <netinet/in.h>


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
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

#define BUFFER_SIZE 256
#include "common.c"

void get(FILE* sockFile, char* path) {
  FILE* inFile = fopen(path, "rb");

  if (inFile == NULL) {
    perror("error reading file");
    return;
  }

  pipeStreams(inFile, sockFile);

  fclose(inFile);
}

void put(FILE* sockFile, char* path) {
  /* FILE* outFile = fopen(path, "w"); */
  FILE* outFile = fopen(path, "wb");

  if (outFile == NULL) {
    perror("error reading file");
    return;
  }

  pipeStreams(sockFile, outFile);
  /* pipeStreams(sockFile, stdout); */

  fclose(outFile);
}

void ls(FILE* sockFile, char* path) {
  DIR *directory = opendir(".");

  if (directory != NULL) {
    struct dirent* entry;

    while ((entry = readdir(directory)) != NULL) {

      if (entry->d_name == NULL) {
        continue;
      }

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

          fprintf(sockFile, "%s %s %d\n",
            (S_ISDIR(statBuffer.st_mode)) ? "directory " : "file ",
            entry->d_name,
            statBuffer.st_size
          );
      }
    }

    closedir(directory);
  }
}


void doprocessing (int sock) {
  int bytesRead;
  char buffer[BUFFER_SIZE];
  bzero(buffer, BUFFER_SIZE);
  FILE* sockFile = fdopen(sock, "r+");

  char* currentPath = ".";

  if (fgets(buffer, BUFFER_SIZE - 1, sockFile) == NULL) {
      perror("ERROR reading from socket");
  }

  for (int i = 0; i < BUFFER_SIZE; i++) {
    if (buffer[i] == '\n') {
      buffer[i] = '\0';
    }
  }

  if (strcmp(buffer, "ls") == 0) {
    printf("ls %s\n", currentPath);
    ls(sockFile, currentPath);
  } else if (strncmp(buffer, "get", 3) == 0) {
    get(sockFile, buffer + 4);
  } else if (strncmp(buffer, "put", 3) == 0) {
    put(sockFile, buffer + 4);
  }

  fclose(sockFile);
  close(sock);
}

int main(int argc, char** argv) {
  int sockfd, newsockfd, portno, clilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  int n, pid;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    perror("ERROR opening socket");
    exit(1);
  }

  /* Initialize socket structure */
  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  /* Now bind the host address using bind() call.*/
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    perror("ERROR on binding");
    exit(1);
  }

  listen(sockfd,5);
  clilen = sizeof(cli_addr);

  while (1) {
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

    if (newsockfd < 0) {
      perror("ERROR on accept");
      exit(1);
    }

    /* Create child process */
    pid = fork();

    if (pid < 0) {
      perror("ERROR on fork");
      exit(1);
    }

    if (pid == 0) {
      close(sockfd);
      doprocessing(newsockfd);
      exit(0);
    }
    else {
      close(newsockfd);
    }
  }
}
