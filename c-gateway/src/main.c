// main.c
#include <stdio.h>
#include <signal.h>
#include <unistd.h>   // sleep
#include <curl/curl.h>

#include "mqtt_client.h"
#include "http_server.h"

static volatile int keep_running = 1;

// --------------- Signal handling ---------------
static void handle_signal(int sig)
{
    (void)sig;
    keep_running = 0;
}

int main(void)
{
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    printf("[GATEWAY] Starting...\n");

    // libcurl global init
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
        fprintf(stderr, "[GATEWAY] curl_global_init failed\n");
        return 1;
    }

    if (!mqtt_init()) {
        fprintf(stderr, "[GATEWAY] MQTT init failed\n");
        curl_global_cleanup();
        return 1;
    }

    if (!http_server_start()) {
        fprintf(stderr, "[GATEWAY] HTTP server init failed\n");
        mqtt_cleanup();
        curl_global_cleanup();
        return 1;
    }

    printf("[GATEWAY] Running. Press Ctrl+C to exit.\n");

    while (keep_running) {
        sleep(1);
    }

    printf("[GATEWAY] Shutting down...\n");
    http_server_stop();
    mqtt_cleanup();
    curl_global_cleanup();

    return 0;
}
