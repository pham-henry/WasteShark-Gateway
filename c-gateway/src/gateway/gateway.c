// gateway.c
// Core gateway module that coordinates all subsystems

// Standard library headers (use angle brackets for system headers)
#include <stdio.h>        // For printf(), fprintf(), stderr - input/output functions
#include <curl/curl.h>    // libcurl library for HTTP client functionality

// Our own header files (use quotes for project headers)
#include "config.h"        // Configuration constants (MQTT_HOST, ports, etc.)
#include "signals.h"       // Signal handling - provides g_keep_running variable
#include "mqtt_client.h"   // MQTT client functions
#include "http_server.h"   // HTTP server functions
#include "gateway.h"       // This module's own header

// Preprocessor directive: conditional compilation based on platform
// #if defined(_WIN32) checks if we're compiling on Windows
// This allows us to write platform-specific code
#if defined(_WIN32)
    // Windows-specific includes and code
    #include <windows.h>   // Windows API header
    // Define a macro that expands to Sleep(1000) - Windows sleep function (milliseconds)
    #define GATEWAY_SLEEP() Sleep(1000)
#else
    // POSIX (Linux, macOS, etc.) code
    #include <unistd.h>    // Unix standard library - provides sleep()
    // Define a macro that expands to sleep(1) - POSIX sleep function (seconds)
    #define GATEWAY_SLEEP() sleep(1)
#endif
// Macros are text replacements done by the preprocessor before compilation
// GATEWAY_SLEEP() will be replaced with the appropriate sleep call

// Initialize all subsystems in the correct order
// Returns 1 (true) on success, 0 (false) on failure
int gateway_init(void)
{
    // printf() prints formatted text to stdout (standard output)
    // \n is the newline character
    printf("[GATEWAY] Starting...\n");

    // Initialize libcurl library globally
    // CURL_GLOBAL_DEFAULT is a constant that sets default initialization flags
    // curl_global_init() returns 0 on success, non-zero on error
    // != 0 means "not equal to zero" (i.e., an error occurred)
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
        // fprintf() is like printf() but allows specifying the output stream
        // stderr is the standard error stream (for error messages)
        fprintf(stderr, "[GATEWAY] curl_global_init failed\n");
        return 0;  // Return 0 to indicate failure
    }

    // Initialize MQTT client and connect to broker
    // !mqtt_init() means "if mqtt_init() returns 0 (false)"
    if (!mqtt_init()) {
        fprintf(stderr, "[GATEWAY] MQTT init failed\n");
        // Clean up what we've initialized so far (libcurl)
        // This prevents resource leaks
        curl_global_cleanup();
        return 0;
    }

    // Start the HTTP server
    if (!http_server_start()) {
        fprintf(stderr, "[GATEWAY] HTTP server init failed\n");
        // Clean up in reverse order: HTTP server, then MQTT, then libcurl
        mqtt_cleanup();
        curl_global_cleanup();
        return 0;
    }

    // All subsystems initialized successfully
    return 1;  // Return 1 (true) to indicate success
}

// Main event loop - runs until shutdown signal is received
// 'void' means this function takes no parameters and returns nothing
void gateway_run(void)
{
    printf("[GATEWAY] Running. Press Ctrl+C to exit.\n");

    // while loop: continues as long as condition is true
    // g_keep_running is a global variable (declared in signals.h)
    // It's set to 0 when a shutdown signal (Ctrl+C) is received
    // volatile keyword ensures the compiler doesn't optimize away checks
    while (g_keep_running) {
        // Sleep for 1 second to avoid busy-waiting (using CPU constantly)
        // The macro expands to the appropriate sleep function for the platform
        GATEWAY_SLEEP();
    }
    // Loop exits when g_keep_running becomes 0
}

// Cleanup function - releases all resources
// Called when program is shutting down
void gateway_shutdown(void)
{
    printf("[GATEWAY] Shutting down...\n");
    
    // Stop the HTTP server (closes listening socket)
    http_server_stop();
    
    // Clean up MQTT client (disconnect, free memory)
    mqtt_cleanup();
    
    // Clean up libcurl (release global resources)
    curl_global_cleanup();
}
