// http_client.h
#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

// Very simple function that sends telemetry JSON to backend.
int send_telemetry_to_backend(const char *json);

#endif // HTTP_CLIENT_H
