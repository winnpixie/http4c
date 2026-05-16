#include "httpsrv.h"
#include "client.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#define BACK_LOG 64

int server_state = HTTP_STOPPED;

int prepare_server(const int fd_server_sock, const int port)
{
	struct sockaddr_in server_addr;
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int reuse_addr_opt = 1;
	setsockopt(fd_server_sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr_opt, sizeof(reuse_addr_opt));

	if (bind(fd_server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
		perror("Error - bind()");
		return -1;
	}

	if (listen(fd_server_sock, BACK_LOG) == -1)
	{
		perror("Error - listen()");
		return -2;
	}

	return 0;
}

int create_server(const int port)
{
	int fd_server_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd_server_sock == -1)
	{
		perror("Error - socket()");
		return -1;
	}

	if (prepare_server(fd_server_sock, port) < 0) {
		perror("Error - configuring");
		return -2;
	}

	return fd_server_sock;
}

void start_server(const int fd_server_sock)
{
	server_state = HTTP_RUNNING;

	while (server_state == HTTP_RUNNING) {
		struct sockaddr_in client_addr;
		socklen_t sz_client_addr = sizeof(struct sockaddr_in);

		int fd_client_sock = accept(fd_server_sock, (struct sockaddr *)&client_addr, &sz_client_addr);
		if (fd_client_sock != -1) {
			handle_client(fd_client_sock);
		}
	}

	stop_server(fd_server_sock);
}

void stop_server(const int fd_server_sock)
{
	shutdown(fd_server_sock, SHUT_RDWR);
	close(fd_server_sock);
}

void set_server_state(const int new_state)
{
	server_state = new_state;
}
