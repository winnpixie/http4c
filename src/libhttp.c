#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void close_socket(const int sockfd)
{
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
}

char *read_line(const int sockfd)
{
	char *line = malloc(sizeof(char) * 1);
	if (line == NULL)
	{
		return NULL;
	}

	line[0] = '\0';

	char buf[1];
	int char_count = 0;
	while (read(sockfd, buf, 1) != -1)
	{
		switch (buf[0])
		{
			case '\r':
				break;
			case '\n':
				return line;
			default:
				line[char_count] = buf[0];
				char_count++;

				char *new_line = malloc(sizeof(char) * (char_count + 1));
				if (new_line == NULL)
				{
					free(line);

					return NULL;
				}

				memcpy(new_line, line, char_count);
				free(line);

				new_line[char_count] = '\0';
				line = new_line;
				break;
		}
	}

	return line;
}

char **split_string(char *str, const char *delim, int *arr_size)
{
	int in_len = strlen(str);
	int split_count = 1;
	for (int i = 0; i < in_len; i++)
	{
		if (str[i] == delim[0])
		{
			split_count++;
		}
	}

	arr_size[0] = split_count;
	char **arr = malloc(sizeof(char) * split_count);
	if (arr == NULL)
	{
		return NULL;
	}

	if (split_count == 1)
	{
		arr[0] = str;

		return arr;
	}


	char *cpy = malloc(sizeof(char) * (in_len + 1));
	if (cpy == NULL)
	{
		free(arr);

		return NULL;
	}
	memcpy(cpy, str, in_len + 1);

	char *tok = strtok(cpy, delim);
	int idx = 0;
	while(tok != NULL)
	{
		int tok_len = strlen(tok);
		char *tokcpy = malloc(sizeof(char) * (tok_len + 1));
		if (tokcpy == NULL)
		{
			free(tok);
			free(cpy);
			free(arr);

			return NULL;
		}

		memcpy(tokcpy, tok, tok_len + 1);

		arr[idx++] = tokcpy;
		tok = strtok(NULL, delim);
	}

	free(tok);
	free(cpy);
	return arr;
}

int main(int argc, char *argv[])
{
	int server_port = 8080;
	if (argc > 1)
	{
		server_port = atoi(argv[1]);
	}

	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1)
	{
		perror("socket");

		return EXIT_FAILURE;
	} else
	{
		printf("Obtained socket @ fd %d.\n", server_socket);
	}

	int yes = 1;
	setsockopt(server_socket, SOCK_STREAM, SO_REUSEADDR, &yes, sizeof(yes));

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
		perror("bind");
		close_socket(server_socket);

		return EXIT_FAILURE;
	}

	printf("Bound to port %d.\n", server_port);

	if (listen(server_socket, 5) == -1)
	{
		perror("listen");
		close_socket(server_socket);
		return EXIT_FAILURE;
	}

	printf("Listening...\n");

	while (1 == 1)
	{
		struct sockaddr client_addr;
		socklen_t client_len = sizeof(client_addr);
		int client_socket = accept(server_socket, &client_addr, &client_len);
		if (client_socket == -1)
		{
			perror("accept");

			break;
		}

		printf("Got connection.\n");

		char *head_line = read_line(client_socket);
		printf("%s\n", head_line);

		int split_size = 0;
		char **head = split_string(head_line, " ", &split_size);
		free(head_line);

		char *line = NULL;
		while ((line = read_line(client_socket)) != NULL)
		{
			printf("%s\n", line);

			if (strlen(line) == 0)
			{
				free(line);

				break;
			}

			free(line);
		}

		char *req_path = head[1];
		if (strlen(req_path) == 1)
		{
			req_path = "/index.html";
		}

		FILE *req_file = fopen(req_path + 1, "r");
		free(head);

		if (req_file != NULL)
		{
			fseek(req_file, 0, SEEK_END);
			long file_size = ftell(req_file);
			rewind(req_file);

			size_t body_size = sizeof(char) * file_size;
			char *body = malloc(body_size);
			if (body == NULL)
			{
				fclose(req_file);
				close_socket(client_socket);

				break;
			}

			fread(body, sizeof(char), file_size, req_file);
			fclose(req_file);

			char ok[] = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n";
			write(client_socket, ok, sizeof(ok) - 1);
			write(client_socket, body, body_size);
		} else
		{
			char not_found[] = "HTTP/1.1 404 Not Found\r\nConnection: close\r\nContent-Type: text/plain\r\n\r\n404.";
			write(client_socket, not_found, sizeof(not_found) - 1);
		}

		close_socket(client_socket);
	}


	close_socket(server_socket);

	return EXIT_SUCCESS;
}
