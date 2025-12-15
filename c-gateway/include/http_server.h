// server.h
#ifndef SERVER_H
#define SERVER_H

// Start HTTP server with its own internal thread
int http_server_start(void);

// Stop HTTP server
void http_server_stop(void);

#endif // HTTP_SERVER_H
