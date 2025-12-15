// gateway.h
#ifndef GATEWAY_H
#define GATEWAY_H

// Initialize everything (curl, MQTT, HTTP server).
// Returns 1 on success, 0 on failure.
int gateway_init(void);

// Main run loop (blocks until g_keep_running == 0).
void gateway_run(void);

// Cleanup resources.
void gateway_shutdown(void);

#endif // GATEWAY_H
