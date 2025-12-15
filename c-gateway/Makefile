
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


# Added for project reuirements, just runs the program and prints instructions before running.
demo: $(TARGET)
	@echo "===================================================="
	@echo "  DEMO MODE"
	@echo "===================================================="
	@echo "Make sure the following are running:"
	@echo "  1) Mosquitto MQTT broker:"
	@echo "       sudo systemctl start mosquitto"
	@echo "  2) Express backend (in express-backend folder):"
	@echo "       npm start"
	@echo "----------------------------------------------------"
	@echo "Starting c_gateway..."
	@echo "----------------------------------------------------"
	./$(TARGET)

# Clean rule
clean:
	rm -f $(TARGET)

# Default target
all: $(TARGET)

