#ifndef HTTPSRV_H
#define HTTPSRV_H

int create_server(const int port);

void start_server(const int fd_server_sock);

void stop_server(const int fd_server_sock);

void set_server_state(const int new_state);

#define HTTP_RUNNING 1
#define HTTP_STOPPED 0
#define HTTP_ERROR -1

#endif // HTTPSRV_H
