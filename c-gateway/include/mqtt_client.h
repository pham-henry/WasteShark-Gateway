// mqtt_client.h
#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

// Initialize MQTT client and subscribe to telemetry topic
int mqtt_init(void);

// Publish JSON command to robot/command topic
int mqtt_publish_command(const char *json);

// Cleanup MQTT client
void mqtt_cleanup(void);

#endif // MQTT_CLIENT_H
