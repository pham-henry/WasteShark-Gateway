// main.c
// This is the entry point of the program - where execution begins

// Include header files that declare functions we'll use
// These are our own header files (note the quotes, not angle brackets)
#include "signals.h"  // Provides setup_signal_handlers() function
#include "gateway.h"  // Provides gateway_init(), gateway_run(), gateway_shutdown()

// main() is the required entry point for all C programs
// 'int' means it returns an integer (0 = success, non-zero = error)
// 'void' means it takes no parameters
int main(void)
{
    // Set up signal handlers so we can gracefully shut down on Ctrl+C
    // This registers functions to be called when the program receives signals
    setup_signal_handlers();

    // Initialize all subsystems (MQTT, HTTP server, libcurl)
    // gateway_init() returns 1 (true) on success, 0 (false) on failure
    // The ! operator negates: if gateway_init() returns 0, the condition is true
    if (!gateway_init()) {
        // Return 1 to indicate program failed to start
        // In C, non-zero return codes typically indicate errors
        return 1;
    }

    // Run the main event loop - this blocks until shutdown signal is received
    // The loop continuously checks for MQTT messages and HTTP requests
    gateway_run();
    
    // Clean up all resources (close connections, free memory, etc.)
    // This is important to prevent resource leaks
    gateway_shutdown();

    // Return 0 to indicate successful program completion
    return 0;
}
