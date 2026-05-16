#ifndef HTTPSRV_H
#define HTTPSRV_H

int create_server(const int);

void start_server(const int);

void stop_server(const int);

void set_server_state(const int);

#define HTTP_RUNNING 1
#define HTTP_STOPPED 0
#define HTTP_ERROR -1

#endif // HTTPSRV_H
