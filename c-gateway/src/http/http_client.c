// http_client.c
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

#include "config.h"
#include "http_client.h"

int send_telemetry_to_backend(const char *json)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "[HTTP CLIENT] curl_easy_init failed\n");
        return 0;
    }

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, BACKEND_URL);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(json));

    printf("[HTTP CLIENT] POST %s\n", BACKEND_URL);
    printf("[HTTP CLIENT] Body: %s\n", json);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "[HTTP CLIENT] Request failed: %s\n",
                curl_easy_strerror(res));
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return 0;
    }

    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    printf("[HTTP CLIENT] Response status: %ld\n", status);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return (status >= 200 && status < 300);
}
