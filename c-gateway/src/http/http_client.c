// http_client.c
// HTTP client module - sends telemetry data to the backend API

#include <stdio.h>        // For printf(), fprintf() - input/output
#include <string.h>       // For strlen() - string length function
#include <curl/curl.h>    // libcurl library for HTTP requests

#include "config.h"       // Contains BACKEND_URL constant
#include "http_client.h"  // This module's header file

// Function to send telemetry JSON data to the backend server
// Parameter: const char *json - pointer to a string containing JSON data
//   'const' means we won't modify the string (read-only)
//   'char *' is a pointer to a character (string in C is array of chars)
// Returns: 1 on success, 0 on failure
int send_telemetry_to_backend(const char *json)
{
    // Variable declarations
    // CURL * is a pointer to a CURL handle (libcurl's internal structure)
    // This handle stores all the settings for an HTTP request
    CURL *curl;
    
    // CURLcode is an enum type from libcurl that represents error codes
    // CURLE_OK means success, other values indicate different errors
    CURLcode res;

    // Initialize a new CURL handle
    // curl_easy_init() allocates memory and returns a pointer
    // Returns NULL if it fails (NULL is a special pointer value meaning "nothing")
    curl = curl_easy_init();
    
    // Check if initialization failed
    // !curl means "if curl is NULL" (the negation operator)
    if (!curl) {
        fprintf(stderr, "[HTTP CLIENT] curl_easy_init failed\n");
        return 0;  // Return 0 to indicate failure
    }

    // Create HTTP headers list
    // curl_slist is a linked list structure for storing HTTP headers
    // NULL means "empty list" or "no list yet"
    struct curl_slist *headers = NULL;
    
    // Add a header to the list
    // curl_slist_append() adds a new header and returns the updated list pointer
    // We need to tell the server we're sending JSON data
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Configure the HTTP request using curl_easy_setopt()
    // This function takes 3 parameters: handle, option, value
    // Set the URL to send the request to (from config.h)
    curl_easy_setopt(curl, CURLOPT_URL, BACKEND_URL);
    
    // Set the HTTP headers we created
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // Set request method to POST (1L means long integer 1, meaning "true")
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    
    // Set the data to send in the POST body (the JSON string)
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
    
    // Set the size of the POST data
    // strlen() returns the length of a string (number of characters)
    // (long) is a type cast - converts size_t to long
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(json));

    // Print debug information
    // %s is a format specifier for strings
    printf("[HTTP CLIENT] POST %s\n", BACKEND_URL);
    printf("[HTTP CLIENT] Body: %s\n", json);

    // Execute the HTTP request
    // curl_easy_perform() actually sends the request and waits for response
    // Returns CURLE_OK (0) on success, error code on failure
    res = curl_easy_perform(curl);
    
    // Check if the request failed
    // != means "not equal to"
    if (res != CURLE_OK) {
        // Print error message
        // curl_easy_strerror() converts error code to human-readable string
        fprintf(stderr, "[HTTP CLIENT] Request failed: %s\n",
                curl_easy_strerror(res));
        
        // Clean up resources before returning
        // curl_slist_free_all() frees all memory used by the headers list
        curl_slist_free_all(headers);
        // curl_easy_cleanup() frees the CURL handle
        curl_easy_cleanup(curl);
        return 0;
    }

    // Get the HTTP response status code
    // 'long' is a data type for integers (larger than int on some systems)
    long status = 0;
    // curl_easy_getinfo() retrieves information about the completed request
    // &status is the address-of operator - passes a pointer to status variable
    // The function will write the status code into this variable
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    printf("[HTTP CLIENT] Response status: %ld\n", status);  // %ld for long integer

    // Clean up resources
    // Always free memory you allocate!
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    // Return 1 if status code is 2xx (success), 0 otherwise
    // >= means "greater than or equal to"
    // < means "less than"
    // This expression evaluates to 1 (true) if status is between 200-299
    return (status >= 200 && status < 300);
}
