// config.h
// Configuration file - contains constants used throughout the program

// Header guards: prevent this file from being included multiple times
// #ifndef means "if not defined" - checks if CONFIG_H has been defined
#ifndef CONFIG_H
// If not defined, define it now
#define CONFIG_H
// This prevents the compiler from processing this file multiple times
// which would cause errors from duplicate definitions

// ---------------- Configuration ----------------

// MQTT broker settings
// #define creates a macro - a text replacement done before compilation
// MQTT_HOST will be replaced with "127.0.0.1" everywhere it's used
// 127.0.0.1 is the loopback address (localhost - same machine)
#define MQTT_HOST "127.0.0.1"   // change this based on PI api, for MQTT testing it should be localhost

// MQTT_PORT is a numeric constant (no quotes = number, not string)
// 1883 is the standard MQTT port
#define MQTT_PORT 1883

// MQTT topic names (strings - note the quotes)
// Topics are like channels - messages are published to and subscribed from topics
#define MQTT_TOPIC_COMMAND   "robot/command"    // Topic for sending commands to robots
#define MQTT_TOPIC_TELEMETRY "robot/telemetry"  // Topic for receiving telemetry from robots

// HTTP server configuration
// Port number where our HTTP server will listen for incoming requests
// Ports are like door numbers - programs listen on specific ports
#define GATEWAY_HTTP_PORT 8000

// Backend API URL
// This is where we send telemetry data via HTTP POST
// "http://localhost:8080/api/telemetry" means:
//   - http:// - protocol
//   - localhost - same machine
//   - 8080 - port number
//   - /api/telemetry - path/endpoint
#define BACKEND_URL "http://localhost:8080/api/telemetry"

// Maximum size for HTTP request bodies (in bytes)
// This limits how much data we'll accept to prevent buffer overflows
// 1024 bytes = 1 kilobyte
#define MAX_BODY_SIZE 1024

// End of header guard
#endif // CONFIG_H
