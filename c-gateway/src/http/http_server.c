// http_server.c
// HTTP server module - receives commands and forwards them to MQTT

#include <stdio.h>        // For printf(), fprintf() - input/output
#include <stdlib.h>       // For malloc(), free() - memory management
#include <string.h>       // For strcmp(), strlen(), memcpy() - string operations
#include <microhttpd.h>   // libmicrohttpd library for HTTP server

#include "config.h"       // Contains MAX_BODY_SIZE, GATEWAY_HTTP_PORT
#include "mqtt_client.h"  // For mqtt_publish_command()
#include "http_server.h" // This module's header

// Define a structure to store data for each HTTP connection
// 'struct' is a way to group related variables together
// This structure stores the POST body data as it arrives in chunks
struct connection_info {
    char body[MAX_BODY_SIZE];  // Buffer to store the JSON command
    size_t size;               // Current number of bytes stored in body
    // size_t is an unsigned integer type for sizes (can't be negative)
};

// Global variable to store the HTTP server daemon handle
// 'static' means this variable is only visible in this file (file scope)
// NULL is a special pointer value meaning "not pointing to anything"
static struct MHD_Daemon *g_daemon = NULL;

// Callback function called by libmicrohttpd for each HTTP request
// This function is called multiple times per request as data arrives
// Parameters:
//   cls - closure data (not used, so we ignore it)
//   connection - the HTTP connection object
//   url - the requested URL path (e.g., "/command")
//   method - HTTP method (e.g., "GET", "POST")
//   version - HTTP version (not used)
//   upload_data - pointer to incoming POST data chunk
//   upload_data_size - pointer to size of the chunk (we modify this)
//   con_cls - pointer to per-connection data (we store our struct here)
static enum MHD_Result http_request_handler(void *cls,
                                struct MHD_Connection *connection,
                                const char *url,
                                const char *method,
                                const char *version,
                                const char *upload_data,
                                size_t *upload_data_size,
                                void **con_cls)
{
    // (void)variable_name tells compiler we're intentionally not using this parameter
    // This prevents compiler warnings about unused variables
    (void)cls;
    (void)version;

    // First call: *con_cls is NULL, so we need to allocate memory for our struct
    // *con_cls is dereferencing the pointer - getting the value it points to
    if (*con_cls == NULL) {
        // malloc() allocates memory from the heap
        // sizeof() returns the size in bytes of a type or variable
        // Returns a pointer to the allocated memory, or NULL if it fails
        struct connection_info *info = malloc(sizeof(struct connection_info));
        
        // Check if malloc failed (returns NULL)
        if (!info) return MHD_NO;  // Return error code
        
        // Initialize the struct fields
        // -> operator accesses a member of a struct through a pointer
        info->size = 0;           // No data received yet
        info->body[0] = '\0';     // Set first character to null terminator (empty string)
        
        // Store our struct pointer in con_cls so we can retrieve it later
        *con_cls = info;  // *con_cls means "the value pointed to by con_cls"
        return MHD_YES;   // Tell library to continue processing
    }

    // Subsequent calls: retrieve our struct from con_cls
    // (struct connection_info *) is a type cast - converts void* to our struct type
    struct connection_info *info = (struct connection_info *)(*con_cls);

    // Check if this is a POST request to /command endpoint
    // strcmp() compares two strings, returns 0 if they're equal
    // && is logical AND - both conditions must be true
    if (strcmp(method, "POST") == 0 && strcmp(url, "/command") == 0) {
        // Check if there's more data to receive
        // *upload_data_size is the size of the current chunk
        if (*upload_data_size != 0) {
            // Append incoming data chunk to our buffer
            // Check if we have room (avoid buffer overflow)
            if (info->size + *upload_data_size < MAX_BODY_SIZE) {
                // memcpy() copies bytes from one memory location to another
                // Parameters: destination, source, number of bytes
                // info->body + info->size is pointer arithmetic - points to end of current data
                memcpy(info->body + info->size, upload_data, *upload_data_size);
                
                // Update the size counter
                info->size += *upload_data_size;  // += is addition assignment
                
                // Add null terminator to make it a valid C string
                info->body[info->size] = '\0';
            } else {
                fprintf(stderr, "[HTTP SERVER] Body too large, dropping\n");
            }

            // Tell library we've consumed all the data in this chunk
            *upload_data_size = 0;
            return MHD_YES;
        } else {
            // All data received - upload_data_size is 0
            // info->body now contains the complete JSON command
            printf("[HTTP SERVER] /command body: %s\n", info->body);

            // Forward the command to MQTT broker
            int ok = mqtt_publish_command(info->body);

            // Ternary operator: condition ? value_if_true : value_if_false
            // If ok is 1 (true), use first string, otherwise use second
            const char *response_text = ok ? "Command accepted\n"
                                           : "Failed to publish command\n";
            int status_code = ok ? MHD_HTTP_OK
                                 : MHD_HTTP_INTERNAL_SERVER_ERROR;

            // Create HTTP response
            // MHD_create_response_from_buffer() creates a response object
            // strlen() gets the length of the response text
            // (void *) is a cast to void pointer (generic pointer type)
            // MHD_RESPMEM_PERSISTENT means the buffer will persist (not freed immediately)
            struct MHD_Response *response =
                MHD_create_response_from_buffer(strlen(response_text),
                                                (void *)response_text,
                                                MHD_RESPMEM_PERSISTENT);
            
            // Queue the response to be sent to the client
            int ret = MHD_queue_response(connection, status_code, response);
            
            // Free the response object (we're done with it)
            MHD_destroy_response(response);

            // Free the connection_info struct we allocated
            free(info);
            *con_cls = NULL;  // Clear the pointer
            return ret;
        }
    }

    // Any other path or method -> return 404 Not Found
    const char *not_found = "Not found\n";
    struct MHD_Response *response =
        MHD_create_response_from_buffer(strlen(not_found),
                                        (void *)not_found,
                                        MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);

    // Clean up
    free(info);
    *con_cls = NULL;
    return ret;
}

// Start HTTP server with its own internal thread
// Returns 1 on success, 0 on failure
int http_server_start(void)
{
    // Start the HTTP server daemon (background process)
    // MHD_start_daemon() takes many parameters:
    //   MHD_USE_INTERNAL_POLLING_THREAD - use a separate thread for network I/O
    //   GATEWAY_HTTP_PORT - port number to listen on (from config.h)
    //   NULL, NULL - access control callbacks (not used)
    //   &http_request_handler - address of our callback function
    //     & is the address-of operator - passes a pointer to the function
    //   NULL - closure data (not used)
    //   MHD_OPTION_END - marks the end of options list
    g_daemon = MHD_start_daemon(
        MHD_USE_INTERNAL_POLLING_THREAD,
        GATEWAY_HTTP_PORT,
        NULL, NULL,
        &http_request_handler,
        NULL,
        MHD_OPTION_END);

    // Check if server started successfully
    // MHD_start_daemon() returns NULL on failure
    if (!g_daemon) {
        fprintf(stderr, "[HTTP SERVER] Failed to start daemon\n");
        return 0;
    }

    // %d is format specifier for integers (decimal)
    printf("[HTTP SERVER] Listening on port %d\n", GATEWAY_HTTP_PORT);
    return 1;  // Success
}

// Stop HTTP server
// 'void' means no parameters and no return value
void http_server_stop(void)
{
    // Check if server is running (g_daemon is not NULL)
    if (g_daemon) {
        // Stop the server and free resources
        MHD_stop_daemon(g_daemon);
        // Set to NULL to indicate server is stopped
        g_daemon = NULL;
    }
}
