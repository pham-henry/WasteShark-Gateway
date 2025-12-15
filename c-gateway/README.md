# C Gateway

A C-based gateway service that bridges MQTT communication with an HTTP backend. The gateway receives robot telemetry via MQTT and forwards it to a backend API, while also accepting HTTP commands and publishing them to the robot via MQTT.

## Architecture

The gateway acts as a bidirectional bridge:
- **MQTT → HTTP**: Receives telemetry from robots via MQTT and forwards to backend
- **HTTP → MQTT**: Accepts commands via HTTP and publishes them to robots via MQTT

## Directory Structure

### Header Files (`include/`)

- **`config.h`** - Configuration constants including MQTT broker settings (host, port, topics), HTTP server port, backend URL, and maximum request body size.

- **`gateway.h`** - Main gateway API providing initialization, run loop, and shutdown functions for the gateway service.

- **`http_client.h`** - HTTP client interface for sending telemetry JSON data to the backend API.

- **`http_server.h`** - HTTP server interface for starting and stopping the server that accepts robot commands.

- **`mqtt_client.h`** - MQTT client interface for initializing the client, publishing commands, and cleanup operations.

- **`signals.h`** - Cross-platform signal handling interface for graceful shutdown on SIGINT/SIGTERM (POSIX) or console control events (Windows).

### Source Files (`src/`)

#### Main Entry Point

- **`main.c`** - Application entry point that initializes libcurl, MQTT client, and HTTP server. Runs the main event loop until interrupted by signals, then performs cleanup.

#### Gateway Module (`src/gateway/`)

- **`gateway.c`** - Core gateway implementation providing unified initialization, run loop, and shutdown functions. Handles cross-platform sleep functionality and coordinates all subsystems.

#### HTTP Module (`src/http/`)

- **`http_client.c`** - HTTP client implementation using libcurl. Sends POST requests with JSON telemetry data to the configured backend URL.

- **`http_server.c`** - HTTP server implementation using libmicrohttpd. Listens for POST requests on `/command` endpoint, extracts JSON payload, and publishes commands to MQTT. Returns 404 for other paths.

#### MQTT Module (`src/mqtt/`)

- **`mqtt_client.c`** - MQTT client implementation using mosquitto library. Connects to MQTT broker, subscribes to telemetry topic, forwards received telemetry to backend via HTTP client, and publishes commands to the robot command topic.

#### Platform Module (`src/platform/`)

- **`signals.c`** - Cross-platform signal handling implementation. Provides Windows-specific console control handler and POSIX signal handlers (SIGINT, SIGTERM) to set the global `g_keep_running` flag for graceful shutdown.

## Data Flow

1. **Telemetry Flow**: Robot → MQTT Broker → Gateway (MQTT Client) → HTTP Client → Backend API
2. **Command Flow**: External Client → HTTP Server → Gateway → MQTT Client → MQTT Broker → Robot

## Dependencies

- **libcurl** - HTTP client functionality
- **libmicrohttpd** - HTTP server functionality
- **mosquitto** - MQTT client library

## Configuration

Edit `include/config.h` to configure:
- MQTT broker host and port
- MQTT topics (command and telemetry)
- HTTP server port
- Backend API URL
- Maximum request body size

