// config.h
#ifndef CONFIG_H
#define CONFIG_H

// ---------------- Configuration ----------------

// MQTT broker running on your Raspberry Pi
#define MQTT_HOST "127.0.0.1"   // change this based on PI api, for MQTT testing it should be localhost
#define MQTT_PORT 1883

#define MQTT_TOPIC_COMMAND   "robot/command"
#define MQTT_TOPIC_TELEMETRY "robot/telemetry"

// HTTP server (this gateway)
#define GATEWAY_HTTP_PORT 8000

// Backend (Express.js) URL that receives telemetry
#define BACKEND_URL "http://localhost:8080/api/telemetry"

// Max size for HTTP request bodies (JSON)
#define MAX_BODY_SIZE 1024

#endif // CONFIG_H
