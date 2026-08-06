#include "httpsrv.h"
#include "request.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#define BACK_LOG 64

int server_state = HTTP_STOPPED;

int prepare_server(const int server_sock, const int port)
{
	struct sockaddr_in server_addr
		= {
			.sin_family = AF_INET,
			.sin_port = htons(port),
			.sin_addr.s_addr = htonl(INADDR_ANY)
		};
	
	int reuse_addr_opt = 1;
	setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr_opt, sizeof(reuse_addr_opt));

	if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
		perror("Error - bind()");
		return -1;
	}

	char addr_str[INET_ADDRSTRLEN];
	if (inet_ntop(AF_INET, &server_addr.sin_addr, addr_str, INET_ADDRSTRLEN) == NULL)
	{
		printf("bind(ERR_ADDR:%d)\n", port);
	} else {
		printf("bind(%s:%d)\n", addr_str, port);
	}

	if (listen(server_sock, BACK_LOG) == -1)
	{
		perror("Error - listen()");
		return -2;
	}

	return 0;
}

int create_server(const int port)
{
	int server_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_sock == -1)
	{
		perror("Error - socket()");
		return -1;
	}

	if (prepare_server(server_sock, port) < 0) {
		perror("Error - configuring");
		return -2;
	}

	return server_sock;
}

void start_server(const int server_sock)
{
	server_state = HTTP_RUNNING;

	while (server_state == HTTP_RUNNING) {
		struct sockaddr_in client_addr;
		socklen_t sz_client_addr = sizeof(struct sockaddr_in);

		int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &sz_client_addr);
		if (client_sock != -1) {
			char addr_str[INET_ADDRSTRLEN];
			if (inet_ntop(AF_INET, &client_addr.sin_addr, addr_str, INET_ADDRSTRLEN) == NULL)
			{
				printf("Request from ERR_ADDR:%d\n", ntohs(client_addr.sin_port));
			} else {
				printf("Request from %s:%d\n", addr_str, ntohs(client_addr.sin_port));
			}

			handle_request(client_sock);
		}
	}

	stop_server(server_sock);
}

void stop_server(const int server_sock)
{
	shutdown(server_sock, SHUT_RDWR);
	close(server_sock);
}

void set_server_state(const int new_state)
{
	server_state = new_state;
}
