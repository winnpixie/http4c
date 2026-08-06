#include "request.h"
#include "utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 65536
#define MAX_HEAD 512

void write_headers(const int client_sock, const int status_code, const char *status_phrase, const char *content_type, const long content_length)
{
	char head[MAX_HEAD];
	
	int len = snprintf(head, MAX_HEAD,
		"HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %ld\r\nConnection: close\r\n\r\n",
		status_code, status_phrase, content_type, content_length
	);
  
	if (len == -1 || len >= MAX_HEAD) {
		return;
	};

	send(client_sock, head, len, 0);
}

void finish_client(const int client_sock)
{
	shutdown(client_sock, SHUT_RDWR);
	close(client_sock);
}

const char *is_valid_http_method(const char *verb)
{
	if (verb == NULL)
	{
		return verb;
	}

	if (strcmp(verb, "GET") == 0 || strcmp(verb, "POST") == 0)
	{
		return verb;
	}

	return NULL;
}

void handle_get(const int client_sock, const char *req_path)
{
	if (req_path == NULL || strcmp(req_path, "/") == 0)
	{
		req_path = "/index.html";
	}

	FILE *fp_req_file = fopen(req_path + 1, "rb");
	if (fp_req_file == NULL) {
		write_headers(client_sock, 404, "Not Found", "text/plain", 4L);
		send(client_sock, "404.", 4, 0);

		return;
	}

	if (fseek(fp_req_file, 0L, SEEK_END) == 0)
	{
		long file_size = ftell(fp_req_file);
		rewind(fp_req_file);

		if (file_size == 0)
		{
			write_headers(client_sock, 200, "OK", "text/plain", 0L);
		} else if (file_size > 0)
		{
			ssize_t resp_size = sizeof(char) * file_size;
			char *resp_buf = malloc(resp_size);
			
			if (resp_buf == NULL)
			{
				write_headers(client_sock, 500, "Internal Server Error", "text/plain", 4L);
				send(client_sock, "500.", 4, 0);
			} else
			{
				size_t resp_read = fread(resp_buf, sizeof(char), file_size, fp_req_file);
				if (resp_read == file_size)
				{
					write_headers(client_sock, 200, "OK", get_content_type(req_path), file_size);
					send(client_sock, resp_buf, file_size, 0);
				}

				free(resp_buf);
			}
		} else
		{
			write_headers(client_sock, 500, "Internal Server Error", "text/plain", 4L);
			send(client_sock, "500.", 4, 0);
		}
	}
		
	fclose(fp_req_file);
}

void handle_request(const int client_sock)
{
	char req_buf[BUF_SIZE];

	ssize_t req_read = recv(client_sock, req_buf, BUF_SIZE - 1, 0);
	if (req_read < 1)
	{
		finish_client(client_sock);

		return;
	}

	req_buf[req_read] = '\0';

	printf("%s\n", req_buf);

	char *p_next_token = NULL;
	char *req_method = strtok_r(req_buf, " ", &p_next_token);
	char *req_path = strtok_r(NULL, " ", &p_next_token);
	
	if (is_valid_http_method(req_method) == NULL)
	{
		write_headers(client_sock, 400, "Bad Request", "text/plain", 4L);
		send(client_sock, "400.", 4, 0);
	} else
	{
		handle_get(client_sock, req_path);
	}

	finish_client(client_sock);
}
