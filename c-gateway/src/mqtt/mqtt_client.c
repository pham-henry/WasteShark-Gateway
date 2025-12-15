// mqtt_client.c
#include <stdio.h>
#include <string.h>
#include <mosquitto.h>

#include "config.h"
#include "http_client.h"
#include "mqtt_client.h"

static struct mosquitto *g_mosq = NULL;

// Called when we receive a message from MQTT broker
static void mqtt_message_callback(struct mosquitto *mosq, void *userdata,
                                  const struct mosquitto_message *msg)
{
    (void)mosq;
    (void)userdata;

    printf("[MQTT] Message on topic '%s'\n", msg->topic);

    // We assume payload is text/JSON
    char buffer[MAX_BODY_SIZE];
    int copy_len = msg->payloadlen;
    if (copy_len >= (int)sizeof(buffer)) {
        copy_len = sizeof(buffer) - 1;
    }

    memcpy(buffer, msg->payload, copy_len);
    buffer[copy_len] = '\0';

    if (strcmp(msg->topic, MQTT_TOPIC_TELEMETRY) == 0) {
        printf("[MQTT] Telemetry received: %s\n", buffer);
        // Forward to backend
        if (!send_telemetry_to_backend(buffer)) {
            fprintf(stderr, "[MQTT] Failed to send telemetry to backend\n");
        }
    } else {
        printf("[MQTT] Topic is not telemetry, ignoring.\n");
    }
}

// Initialize MQTT client and subscribe to telemetry topic
int mqtt_init(void)
{
    int rc;

    mosquitto_lib_init();

    g_mosq = mosquitto_new("c_gateway", true, NULL);
    if (!g_mosq) {
        fprintf(stderr, "[MQTT] mosquitto_new failed\n");
        return 0;
    }

    mosquitto_message_callback_set(g_mosq, mqtt_message_callback);

    printf("[MQTT] Connecting to %s:%d\n", MQTT_HOST, MQTT_PORT);
    rc = mosquitto_connect(g_mosq, MQTT_HOST, MQTT_PORT, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "[MQTT] Connect failed: %s\n",
                mosquitto_strerror(rc));
        return 0;
    }

    // Start background thread that handles network traffic
    rc = mosquitto_loop_start(g_mosq);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "[MQTT] loop_start failed: %s\n",
                mosquitto_strerror(rc));
        return 0;
    }

    // Subscribe to telemetry topic
    rc = mosquitto_subscribe(g_mosq, NULL, MQTT_TOPIC_TELEMETRY, 1);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "[MQTT] subscribe failed: %s\n",
                mosquitto_strerror(rc));
        return 0;
    }

    printf("[MQTT] Subscribed to %s\n", MQTT_TOPIC_TELEMETRY);
    return 1;
}

// Publish JSON command to robot/command topic
int mqtt_publish_command(const char *json)
{
    if (!g_mosq) {
        fprintf(stderr, "[MQTT] Not initialized\n");
        return 0;
    }

    int rc = mosquitto_publish(g_mosq, NULL, MQTT_TOPIC_COMMAND,
                               (int)strlen(json), json, 1, false);

    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "[MQTT] publish failed: %s\n",
                mosquitto_strerror(rc));
        return 0;
    }

    printf("[MQTT] Published to %s: %s\n", MQTT_TOPIC_COMMAND, json);
    return 1;
}

// Cleanup MQTT
void mqtt_cleanup(void)
{
    if (g_mosq) {
        mosquitto_loop_stop(g_mosq, true);
        mosquitto_disconnect(g_mosq);
        mosquitto_destroy(g_mosq);
        g_mosq = NULL;
    }
    mosquitto_lib_cleanup();
}
