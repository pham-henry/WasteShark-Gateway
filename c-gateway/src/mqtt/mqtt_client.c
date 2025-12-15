// mqtt_client.c
// MQTT client module - connects to MQTT broker, receives telemetry, publishes commands

#include <stdio.h>        // For printf(), fprintf()
#include <string.h>       // For strcmp(), strlen(), memcpy()
#include <mosquitto.h>    // mosquitto MQTT library

#include "config.h"       // Contains MQTT_HOST, MQTT_PORT, topic names
#include "http_client.h"  // For send_telemetry_to_backend()
#include "mqtt_client.h"  // This module's header

// Global variable to store the MQTT client handle
// 'static' means this variable is only visible in this file
// NULL means "not initialized yet"
static struct mosquitto *g_mosq = NULL;

// Callback function called by mosquitto library when a message is received
// This is registered with mosquitto and called automatically
// Parameters:
//   mosq - the mosquitto client handle (not used)
//   userdata - user data pointer (not used)
//   msg - pointer to the received message structure
static void mqtt_message_callback(struct mosquitto *mosq, void *userdata,
                                  const struct mosquitto_message *msg)
{
    // Mark unused parameters to avoid compiler warnings
    (void)mosq;
    (void)userdata;

    // msg->topic is a pointer to the topic name string
    // %s format specifier prints the string
    printf("[MQTT] Message on topic '%s'\n", msg->topic);

    // Create a buffer to store the message payload (data)
    // We assume the payload is text/JSON, not binary data
    char buffer[MAX_BODY_SIZE];
    
    // msg->payloadlen is the length of the payload in bytes
    int copy_len = msg->payloadlen;
    
    // Safety check: make sure we don't overflow our buffer
    // sizeof(buffer) gives the size of the array in bytes
    // (int) is a type cast - converts size_t to int
    if (copy_len >= (int)sizeof(buffer)) {
        // If payload is too large, only copy what fits (leave room for null terminator)
        copy_len = sizeof(buffer) - 1;
    }

    // Copy the payload data into our buffer
    // msg->payload is a pointer to the actual data (void* type)
    // memcpy() copies bytes from source to destination
    memcpy(buffer, msg->payload, copy_len);
    
    // Add null terminator to make it a valid C string
    // This is essential - C strings must end with '\0'
    buffer[copy_len] = '\0';

    // Check if this message is on the telemetry topic
    // strcmp() returns 0 if strings are equal
    if (strcmp(msg->topic, MQTT_TOPIC_TELEMETRY) == 0) {
        printf("[MQTT] Telemetry received: %s\n", buffer);
        
        // Forward the telemetry data to the backend via HTTP
        // send_telemetry_to_backend() returns 1 on success, 0 on failure
        if (!send_telemetry_to_backend(buffer)) {
            fprintf(stderr, "[MQTT] Failed to send telemetry to backend\n");
        }
    } else {
        // Ignore messages on other topics
        printf("[MQTT] Topic is not telemetry, ignoring.\n");
    }
}

// Initialize MQTT client and subscribe to telemetry topic
// Returns 1 on success, 0 on failure
int mqtt_init(void)
{
    // 'int rc' declares a variable to store return codes
    // rc will hold error codes from mosquitto functions
    int rc;

    // Initialize the mosquitto library (must be called first)
    mosquitto_lib_init();

    // Create a new mosquitto client instance
    // Parameters:
    //   "c_gateway" - client ID (unique identifier for this client)
    //   true - clean session flag (true = start fresh, false = resume previous session)
    //   NULL - user data pointer (not used)
    // Returns a pointer to the client, or NULL on failure
    g_mosq = mosquitto_new("c_gateway", true, NULL);
    if (!g_mosq) {
        fprintf(stderr, "[MQTT] mosquitto_new failed\n");
        return 0;
    }

    // Register our callback function to be called when messages arrive
    // &mqtt_message_callback is the address of our function
    mosquitto_message_callback_set(g_mosq, mqtt_message_callback);

    // Print connection info
    // %s is for strings, %d is for integers
    printf("[MQTT] Connecting to %s:%d\n", MQTT_HOST, MQTT_PORT);
    
    // Connect to the MQTT broker
    // Parameters:
    //   g_mosq - client handle
    //   MQTT_HOST - broker address (from config.h)
    //   MQTT_PORT - broker port (from config.h)
    //   60 - keepalive interval in seconds (how often to send ping)
    // Returns MOSQ_ERR_SUCCESS (0) on success, error code on failure
    rc = mosquitto_connect(g_mosq, MQTT_HOST, MQTT_PORT, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        // mosquitto_strerror() converts error code to readable string
        fprintf(stderr, "[MQTT] Connect failed: %s\n",
                mosquitto_strerror(rc));
        return 0;
    }

    // Start background thread that handles network traffic
    // This allows the MQTT client to receive messages asynchronously
    // The thread runs in the background and calls our callback when messages arrive
    rc = mosquitto_loop_start(g_mosq);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "[MQTT] loop_start failed: %s\n",
                mosquitto_strerror(rc));
        return 0;
    }

    // Subscribe to the telemetry topic
    // Parameters:
    //   g_mosq - client handle
    //   NULL - message ID pointer (not used)
    //   MQTT_TOPIC_TELEMETRY - topic name to subscribe to (from config.h)
    //   1 - quality of service level (0=at most once, 1=at least once, 2=exactly once)
    rc = mosquitto_subscribe(g_mosq, NULL, MQTT_TOPIC_TELEMETRY, 1);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "[MQTT] subscribe failed: %s\n",
                mosquitto_strerror(rc));
        return 0;
    }

    printf("[MQTT] Subscribed to %s\n", MQTT_TOPIC_TELEMETRY);
    return 1;  // Success
}

// Publish JSON command to robot/command topic
// Parameter: const char *json - pointer to JSON string to publish
// Returns 1 on success, 0 on failure
int mqtt_publish_command(const char *json)
{
    // Check if MQTT client is initialized
    // If g_mosq is NULL, we haven't initialized yet
    if (!g_mosq) {
        fprintf(stderr, "[MQTT] Not initialized\n");
        return 0;
    }

    // Publish a message to the MQTT broker
    // Parameters:
    //   g_mosq - client handle
    //   NULL - message ID pointer (not used)
    //   MQTT_TOPIC_COMMAND - topic to publish to (from config.h)
    //   (int)strlen(json) - length of the message in bytes
    //     strlen() returns size_t, we cast to int for the function
    //   json - pointer to the message data
    //   1 - quality of service level (at least once delivery)
    //   false - retain flag (false = don't keep message for new subscribers)
    int rc = mosquitto_publish(g_mosq, NULL, MQTT_TOPIC_COMMAND,
                               (int)strlen(json), json, 1, false);

    // Check if publish succeeded
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "[MQTT] publish failed: %s\n",
                mosquitto_strerror(rc));
        return 0;
    }

    printf("[MQTT] Published to %s: %s\n", MQTT_TOPIC_COMMAND, json);
    return 1;  // Success
}

// Cleanup MQTT client - releases all resources
// Called when program is shutting down
void mqtt_cleanup(void)
{
    // Check if client exists before trying to clean it up
    if (g_mosq) {
        // Stop the background network thread
        // true means "force stop" (don't wait for current operations)
        mosquitto_loop_stop(g_mosq, true);
        
        // Disconnect from the broker
        mosquitto_disconnect(g_mosq);
        
        // Free the client handle and associated memory
        mosquitto_destroy(g_mosq);
        
        // Set to NULL to indicate client is destroyed
        g_mosq = NULL;
    }
    
    // Clean up the mosquitto library (must be called last)
    mosquitto_lib_cleanup();
}
