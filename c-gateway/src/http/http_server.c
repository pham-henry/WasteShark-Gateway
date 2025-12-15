// http_server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>

#include "config.h"
#include "mqtt_client.h"
#include "http_server.h"

// Per-connection info (for accumulating POST data)
struct connection_info {
    char body[MAX_BODY_SIZE];
    size_t size;
};

static struct MHD_Daemon *g_daemon = NULL;

// This function is called multiple times for a request
static enum MHD_Result http_request_handler(void *cls,
                                struct MHD_Connection *connection,
                                const char *url,
                                const char *method,
                                const char *version,
                                const char *upload_data,
                                size_t *upload_data_size,
                                void **con_cls)
{
    (void)cls;
    (void)version;

    // First call: setup connection_info
    if (*con_cls == NULL) {
        struct connection_info *info = malloc(sizeof(struct connection_info));
        if (!info) return MHD_NO;
        info->size = 0;
        info->body[0] = '\0';
        *con_cls = info;
        return MHD_YES;
    }

    struct connection_info *info = (struct connection_info *)(*con_cls);

    // We only handle POST /command
    if (strcmp(method, "POST") == 0 && strcmp(url, "/command") == 0) {
        if (*upload_data_size != 0) {
            // Append incoming data to buffer
            if (info->size + *upload_data_size < MAX_BODY_SIZE) {
                memcpy(info->body + info->size, upload_data, *upload_data_size);
                info->size += *upload_data_size;
                info->body[info->size] = '\0';
            } else {
                fprintf(stderr, "[HTTP SERVER] Body too large, dropping\n");
            }

            *upload_data_size = 0;
            return MHD_YES;
        } else {
            // Upload done; info->body contains full JSON
            printf("[HTTP SERVER] /command body: %s\n", info->body);

            int ok = mqtt_publish_command(info->body);

            const char *response_text = ok ? "Command accepted\n"
                                           : "Failed to publish command\n";
            int status_code = ok ? MHD_HTTP_OK
                                 : MHD_HTTP_INTERNAL_SERVER_ERROR;

            struct MHD_Response *response =
                MHD_create_response_from_buffer(strlen(response_text),
                                                (void *)response_text,
                                                MHD_RESPMEM_PERSISTENT);
            int ret = MHD_queue_response(connection, status_code, response);
            MHD_destroy_response(response);

            free(info);
            *con_cls = NULL;
            return ret;
        }
    }

    // Any other path -> 404
    const char *not_found = "Not found\n";
    struct MHD_Response *response =
        MHD_create_response_from_buffer(strlen(not_found),
                                        (void *)not_found,
                                        MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);

    free(info);
    *con_cls = NULL;
    return ret;
}

// Start HTTP server with its own internal thread
int http_server_start(void)
{
    g_daemon = MHD_start_daemon(
        MHD_USE_INTERNAL_POLLING_THREAD,
        GATEWAY_HTTP_PORT,
        NULL, NULL,
        &http_request_handler,
        NULL,
        MHD_OPTION_END);

    if (!g_daemon) {
        fprintf(stderr, "[HTTP SERVER] Failed to start daemon\n");
        return 0;
    }

    printf("[HTTP SERVER] Listening on port %d\n", GATEWAY_HTTP_PORT);
    return 1;
}

// Stop HTTP server
void http_server_stop(void)
{
    if (g_daemon) {
        MHD_stop_daemon(g_daemon);
        g_daemon = NULL;
    }
}
