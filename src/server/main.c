#include "httpsrv.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// FIXME: Surely there exists a not as dumb looking solution?
void *run_server_thread(void *args)
{
	start_server(((int *)args)[0]);
	
	free(args);

	return NULL;
}

int main(int argc, char *argv[])
{
	int srvport = 8080;
	if (argc > 1)
	{
		srvport = atoi(argv[1]);
	}

	int srvsockfd = create_server(srvport);
	if (srvsockfd < 0)
	{
		printf("Error creating HTTP server on port %d\n", srvport);
		stop_server(srvsockfd);
		
		return EXIT_FAILURE;
	}

	printf("Starting HTTP server on port %d\n", srvport);

	pthread_t srvthread;
	int *thrdargs = malloc(sizeof(int));
	if (thrdargs == NULL)
	{
		perror("Server thread start");
		stop_server(srvsockfd);

		return EXIT_FAILURE;
	}
	
	thrdargs[0] = srvsockfd;
	pthread_create(&srvthread, NULL, run_server_thread, thrdargs);

	while (getchar() != 'x')
	{
		// busy wait
	}
	printf("Stop requested\n");

	set_server_state(HTTP_STOPPED);
	pthread_join(srvthread, NULL);

	printf("Exiting.\n");

	return EXIT_SUCCESS;
}
