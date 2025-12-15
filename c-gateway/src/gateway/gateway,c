 // gateway.c
#include <stdio.h>
#include <curl/curl.h>

#include "config.h"
#include "signals.h"
#include "mqtt_client.h"
#include "http_server.h"
#include "gateway.h"

// Cross-platform sleep macro for the main loop
#if defined(_WIN32)
#include <windows.h>
#define GATEWAY_SLEEP() Sleep(1000)
#else
#include <unistd.h>
#define GATEWAY_SLEEP() sleep(1)
#endif

int gateway_init(void)
{
    printf("[GATEWAY] Starting...\n");

    // libcurl global init
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
        fprintf(stderr, "[GATEWAY] curl_global_init failed\n");
        return 0;
    }

    if (!mqtt_init()) {
        fprintf(stderr, "[GATEWAY] MQTT init failed\n");
        curl_global_cleanup();
        return 0;
    }

    if (!http_server_start()) {
        fprintf(stderr, "[GATEWAY] HTTP server init failed\n");
        mqtt_cleanup();
        curl_global_cleanup();
        return 0;
    }

    return 1;
}

void gateway_run(void)
{
    printf("[GATEWAY] Running. Press Ctrl+C to exit.\n");

    while (g_keep_running) {
        GATEWAY_SLEEP();
    }
}

void gateway_shutdown(void)
{
    printf("[GATEWAY] Shutting down...\n");
    http_server_stop();
    mqtt_cleanup();
    curl_global_cleanup();
}
