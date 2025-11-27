#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int configure_server(int server_fd, int port) {
  struct sockaddr_in srv_addr;

  srv_addr.sin_family = AF_INET;
  srv_addr.sin_port = htons(port);
  srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(server_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) == -1) {
    return 1;
  }

  if (listen(server_fd, 5) == -1) {
    return 1;
  }

  return 0;
}

char **strspl(const char *str, const char delim) {
  char **arr = malloc(1);
  int len = sizeof(str) - 1;
  int loc = 0;

  for (int i = 0; i < len; i++) {
    if (str[i] != delim) {
    }
  }

  return arr;
}

int process_client(int client_fd) {
  printf("client connected.\n");

  size_t bufsize = sizeof(char) * 1024;
  char *buf = malloc(bufsize);
  recv(client_fd, buf, bufsize, 0);

  char *rbuf = strdup(buf);

  char *method = strtok(buf, " ");
  if (strcmp(method, "GET") == 0) {
    printf("method : %s\n", method);

    char *path = strtok(buf + 4, " ");
    if (strlen(path) == 1) {
      path = "/index.html";
    }

    printf("path : %s\n", path);
    FILE *fd = fopen(path + 1, "r");
    if (fd != NULL) {
      fseek(fd, 0, SEEK_END);
      size_t fsize = sizeof(char) * ftell(fd);
      rewind(fd);
      char *fbuf = malloc(fsize);
      fread(fbuf, sizeof(char), fsize, fd);

      char *mesg =
          "HTTP/1.1 200 OK\nContent-Type: text/html\nConnection: close\n\n";
      send(client_fd, mesg, strlen(mesg), 0);
      send(client_fd, fbuf, fsize, 0);

      free(fbuf);
      fclose(fd);
    } else {
      char mesg[] = "HTTP/1.1 404 Not Found\nContent-Type: "
                    "text/html\nConnection: close\n\nNF";
      send(client_fd, mesg, sizeof(mesg), 0);

      printf("NO FILE FOUND '%s'\n", path);
    }
  } else {
    printf("Malformed request.\n");
  }

  free(buf);
  shutdown(client_fd, SHUT_RDWR);
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