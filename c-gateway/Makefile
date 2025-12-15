# Simple Makefile for the c_gateway project

CC = gcc
CFLAGS = -Wall -std=c11 -Iinclude

# Libraries needed:
#   -lmosquitto    (MQTT)
#   -lmicrohttpd   (HTTP server)
#   -lcurl         (HTTP client)
LIBS = -lmosquitto -lmicrohttpd -lcurl

TARGET = c_gateway

# All source files
SRCS = \
    src/main.c \
    src/gateway/gateway.c \
    src/platform/signals.c \
    src/mqtt/mqtt_client.c \
    src/http/http_client.c \
    src/http/http_server.c

# Build rule
$(TARGET):
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

# Clean rule
clean:
	rm -f $(TARGET)

# Default target
all: $(TARGET)

