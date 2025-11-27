#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int configure_server(int srvsockfd, int srvport) {
  struct sockaddr_in srv_addr;

  srv_addr.sin_family = AF_INET;
  srv_addr.sin_port = htons(srvport);
  srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(srvsockfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) == -1) {
    return 1;
  }

  if (listen(srvsockfd, 5) == -1) {
    return 1;
  }

  return 0;
}

// TODO: String split method.
char **strspl(const char *str, const char delim);

int process_client(int clsockfd) {
  printf("Client connected.\n");

  size_t bufsize = sizeof(char) * 1024;
  char *buf = malloc(bufsize);
  recv(clsockfd, buf, bufsize, 0);

  char *reqmethod = strtok(buf, " ");
  if (strcmp(reqmethod, "GET") == 0) {
    printf("Method: %s\n", reqmethod);

    char *reqpath = strtok(buf + 4, " ");
    if (strlen(reqpath) == 1) {
      reqpath = "/index.html";
    }

    printf("Path: %s\n", reqpath);
    FILE *reqfile = fopen(reqpath + 1, "r");
    if (reqfile == NULL) {
      char *resp = "HTTP/1.1 404 Not Found\nContent-Type: text/html\nConnection: close\n\n404.";
      send(clsockfd, resp, strlen(resp), 0);

      printf("404 - '%s'\n", reqpath);
    } else {
      fseek(reqfile, 0, SEEK_END);
      size_t filesize = sizeof(char) * ftell(reqfile);
      rewind(reqfile);

      char *filebuf = malloc(filesize);
      fread(filebuf, sizeof(char), filesize, reqfile);

      fclose(reqfile);

      char *resphead = "HTTP/1.1 200 OK\nContent-Type: text/html\nConnection: close\n\n";
      send(clsockfd, resphead, strlen(resphead), 0);
      send(clsockfd, filebuf, filesize, 0);

      free(filebuf);
    }
  } else {
    printf("Malformed request.\n");
  }

  free(buf);
  shutdown(clsockfd, SHUT_RDWR);
  return 0;
}

int main(int argc, char **argv) {
  int port = 8080;
  if (argc > 1) {
    port = atoi(argv[1]);
  }

  int server_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (server_fd == -1) {
    printf("Error creating socket\n");
    return EXIT_FAILURE;
  }

  if (configure_server(server_fd, port) != 0) {
    printf("Error preparing server.\n");
    return EXIT_FAILURE;
  }

  printf("http server started on port %d\n", port);

  while (true) {
    struct sockaddr client_addr;
    socklen_t client_addr_len;

    int client_fd = accept(server_fd, &client_addr, &client_addr_len);
    if (client_fd != -1) {
      process_client(client_fd);
    }
  }

  shutdown(server_fd, SHUT_RDWR);

  printf("exiting.\n");
  return EXIT_SUCCESS;
}