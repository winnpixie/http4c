#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 65536
#define MAX_HEAD 512

void write_headers(const int fd_client_sock, const char *http_status, const char *content_type, const long content_length)
{
	char head[MAX_HEAD];
	
	int len = snprintf(head, MAX_HEAD,
		"HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %ld\r\nConnection: close\r\n\r\n",
		http_status, content_type, content_length
	);
  
	if (len == -1 || len >= MAX_HEAD) {
		return;
	};

	send(fd_client_sock, head, len, 0);
}

void finish_client(const int fd_client_sock)
{
	shutdown(fd_client_sock, SHUT_RDWR);
	close(fd_client_sock);
}

int is_valid_http_method(const char *verb)
{
	if (verb == NULL)
	{
		return 0;
	}

	if (strcmp(verb, "GET") == 0 || strcmp(verb, "POST") == 0)
	{
		return 1;
	}

	return 0;
}

void handle_get(const int fd_client_sock, const char *req_path)
{
	if (req_path == NULL || strcmp(req_path, "/") == 0)
	{
		req_path = "/index.html";
	}

	FILE *fp_req_file = fopen(req_path + 1, "rb");
	if (fp_req_file == NULL) {
		write_headers(fd_client_sock, "404 Not Found", "text/plain", 4);
		send(fd_client_sock, "404.", 4, 0);

		return;
	}

	if (fseek(fp_req_file, 0L, SEEK_END) == 0)
	{
		long file_size = ftell(fp_req_file);
		rewind(fp_req_file);

		if (file_size == 0)
		{
			write_headers(fd_client_sock, "200 OK", "text/plain", 0L);
		} else if (file_size > 0)
		{
			ssize_t resp_size = sizeof(char) * file_size;
			char *resp_buf = malloc(resp_size);
			
			if (resp_buf == NULL)
			{
				write_headers(fd_client_sock, "500 Internal Server Error", "text/plain", 4);
				send(fd_client_sock, "500.", 4, 0);
			} else
			{
				size_t resp_read = fread(resp_buf, sizeof(char), file_size, fp_req_file);
				if (resp_read == file_size)
				{
					write_headers(fd_client_sock, "200 OK", "text/html", file_size);
					send(fd_client_sock, resp_buf, file_size, 0);
				}

				free(resp_buf);
			}
		} else
		{
			write_headers(fd_client_sock, "500 Internal Server Error", "text/plain", 4L);
			send(fd_client_sock, "500.", 4, 0);
		}
	}
		
	fclose(fp_req_file);
}

void handle_client(const int fd_client_sock)
{
	char req_buf[BUF_SIZE];

	ssize_t req_read = recv(fd_client_sock, req_buf, BUF_SIZE - 1, 0);
	if (req_read < 1)
	{
		finish_client(fd_client_sock);

		return;
	}

	req_buf[req_read] = '\0';

	printf("%s\n", req_buf);

	char *p_next_token = NULL;
	char *req_method = strtok_r(req_buf, " ", &p_next_token);
	char *req_path = strtok_r(NULL, " ", &p_next_token);
	
	if (is_valid_http_method(req_method))
	{
		handle_get(fd_client_sock, req_path);
	} else
	{
		write_headers(fd_client_sock, "400 Bad Request", "text/plain", 4L);
		send(fd_client_sock, "400.", 4, 0);
	}

	finish_client(fd_client_sock);
}
