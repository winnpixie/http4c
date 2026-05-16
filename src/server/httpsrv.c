#include "httpsrv.h"
#include "client.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

int state = HTTP_STOPPED;

int prepare_server(const int srvsockfd, const int srvport)
{
	struct sockaddr_in srvaddr;
	
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(srvport);
	srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int reuseaddropt = 1;
	setsockopt(srvsockfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddropt, sizeof(reuseaddropt));

	if (bind(srvsockfd, (struct sockaddr *)&srvaddr, sizeof(srvaddr)) == -1)
	{
		perror("Error - bind()");
		return -1;
	}

	if (listen(srvsockfd, 5) == -1)
	{
		perror("Error - listen()");
		return -2;
	}

	return 0;
}

int create_server(const int port)
{
	int srvsockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (srvsockfd == -1)
	{
		perror("Error - socket()");
		return -1;
	}

	if (prepare_server(srvsockfd, port) < 0) {
		perror("Error - configuring");
		return -2;
	}

	return srvsockfd;
}

void start_server(const int srvsockfd)
{
	state = HTTP_RUNNING;

	while (state == HTTP_RUNNING) {
		struct sockaddr_in claddr;
		socklen_t claddrlen = sizeof(struct sockaddr_in);

		int clsockfd = accept(srvsockfd, (struct sockaddr *)&claddr, &claddrlen);
		if (clsockfd != -1) {
			handle_client(clsockfd);
		}
	}

	stop_server(srvsockfd);
}

void stop_server(const int srvsockfd)
{
	shutdown(srvsockfd, SHUT_RDWR);
	close(srvsockfd);
}

void set_server_state(const int newstate)
{
	state = newstate;
}
