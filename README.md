# WasteShark Gateway – Run & Test Guide

This document explains **everything you need to install**, **how to run** the system, and **how to test** that the full pipeline works end-to-end.

* **Express.js backend** (HTTP API)
* **C gateway** (HTTP ⇄ MQTT bridge)
* **Mosquitto MQTT broker** (message bus for the robot)

IMPORTANT: This project  assumes you’re running on a **Linux machine (Ubuntu/Debian)** and running **all components locally on the same machine**.

---

## 1. Project Overview

High level architecture (all on one machine):

```text
[Express Backend]  <----HTTP---->  [C Gateway]  <----MQTT---->  [Mosquitto Broker]
      ^                                                          ^
      |                                                          |
   curl / frontend                                       Robot / test clients
```

**Message directions:**

* **Commands**: Backend → Gateway → MQTT → Robot

  * Backend sends `start`/`stop` → Gateway publishes to `robot/command` topic.
* **Telemetry**: Robot → MQTT → Gateway → Backend

  * Robot publishes status → Gateway forwards JSON to backend `/api/telemetry`.

---

## 2. What Needs to Be Installed

### 2.1 System tools (compiler & basics)

Install build tools:

```bash
sudo apt update
sudo apt install -y build-essential
```

### 2.2 MQTT broker & clients (Mosquitto)

```bash
sudo apt install -y mosquitto mosquitto-clients
sudo systemctl enable mosquitto
sudo systemctl start mosquitto
```

This installs:

* **MQTT broker** (`mosquitto`) on port `1883`
* **MQTT test tools**: `mosquitto_pub`, `mosquitto_sub`

### 2.3 C libraries for the gateway

```bash
sudo apt install -y \
  libcurl4-openssl-dev \
  libmicrohttpd-dev \
  libmosquitto-dev
```

These provide:

* `curl/curl.h` + libcurl (HTTP client)
* `microhttpd.h` + libmicrohttpd (HTTP server)
* libmosquitto (MQTT client library)

### 2.4 Node.js & npm for Express backend

If you don’t already have Node.js:

```bash
sudo apt install -y nodejs npm
```

---

## 3. Project Layout

This is our C gateway project:
```text
project-root/
├── Makefile
├── include/
│   ├── config.h
│   ├── gateway.h
│   ├── signals.h
│   ├── http_client.h
│   ├── mqtt_client.h
│   ├── http_server.h
│
└── src/
    ├── main.c
    │
    ├── gateway/
    │   └── gateway.c
    │
    ├── mqtt/
    │   └── mqtt_client.c
    │
    ├── http/
    │   ├── http_client.c
    │   └── http_server.c
    │
    └── platform/
        └── signals.c
```

This is our test express.js http backend:

```text
express-backend/
├── package.json
├── server.js
└── .env
```

---

## 4. Configuration

### 4.1 `include/config.h` (for C gateway)

Make sure this file has these values (for fully local setup):

```c
// config.h

#ifndef CONFIG_H
#define CONFIG_H

// MQTT broker on same machine
#define MQTT_HOST "localhost"
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
```

### 4.2 `.env` (for Express backend)

In `express-backend/.env`:

```env
PORT=8080
GATEWAY_URL=http://localhost:8000/command
```

* Backend listens on `http://localhost:8080`
* Backend forwards commands to the C gateway at `http://localhost:8000/command`

---

## 5. Building the C Gateway

From the **C project root** (where the `Makefile` is):

```bash
make
```

This compiles all `src/*.c` files into an executable named:

```text
c_gateway
```

If you need to clean and rebuild:

```bash
make clean
make
```

---

## 6. Running Everything (Fully Local)

You’ll typically use **three terminals**:

### 6.1 Terminal 1 – Make sure Mosquitto is running

If you already enabled it earlier, it should be running. To be safe:

```bash
sudo systemctl start mosquitto
sudo systemctl status mosquitto
```

You don’t need to leave this terminal open if the service is managed by systemd.

---

### 6.2 Terminal 2 – Run Express backend

Go to your backend folder:

```bash
cd express-backend
npm install      # first time only
npm start
```

You should see something like:

```text
[BACKEND] Starting server...
[BACKEND] Gateway URL: http://localhost:8000/command
[BACKEND] Listening on http://localhost:8080
```

**Optional health check** (in another terminal):

```bash
curl http://localhost:8080/health
```

Expected response (or similar):

```json
{"status":"ok","gateway":"http://localhost:8000/command"}
```

---

### 6.3 Terminal 3 – Run C gateway

From your C project root:

```bash
./c_gateway
```

Expected output:

```text
[GATEWAY] Starting...
[MQTT] Connecting to localhost:1883
[MQTT] Subscribed to robot/telemetry
[HTTP SERVER] Listening on port 8000
[GATEWAY] Running. Press Ctrl+C to exit.
```

Now the whole system is running:

* Mosquitto broker
* Express backend
* C gateway

---

## 7. Testing the System

### 7.1 Test 1 – Command flow (backend → gateway → MQTT)

We’ll simulate a “robot” by subscribing to the `robot/command` topic.

#### 7.1.1 Open a “robot” subscriber (Terminal 4)

```bash
mosquitto_sub -h localhost -t robot/command -q 1
```

Keep this running. It will print any command the gateway publishes.

#### 7.1.2 Send a start command from backend

In **another terminal**, run:

```bash
curl -X POST http://localhost:8080/api/start
```

#### 7.1.3 What you should see

* **Robot subscriber (mosquitto_sub) terminal:**

  ```json
  {"action":"start"}
  ```

* **Gateway terminal:**

  ```text
  [HTTP SERVER] /command body: {"action":"start"}
  [MQTT] Published to robot/command: {"action":"start"}
  ```

* **Backend terminal:**

  ```text
  [BACKEND] Forwarding 'start' to gateway...
  ```

You can also test `stop`:

```bash
curl -X POST http://localhost:8080/api/stop
```

Expect to see:

```json
{"action":"stop"}
```

in the `mosquitto_sub` terminal.

If this works, the **command path is good** ✅

---

### 7.2 Test 2 – Telemetry flow (MQTT → gateway → backend)

Now we simulate the robot sending telemetry.

#### 7.2.1 Publish telemetry via MQTT

In a new terminal:

```bash
mosquitto_pub -h localhost -t robot/telemetry \
  -m '{"status":"cleaning","battery":82}'
```

#### 7.2.2 What you should see

* **Gateway terminal:**

  ```text
  [MQTT] Message on topic 'robot/telemetry'
  [MQTT] Telemetry received: {"status":"cleaning","battery":82}
  [HTTP CLIENT] POST http://localhost:8080/api/telemetry
  [HTTP CLIENT] Body: {"status":"cleaning","battery":82}
  [HTTP CLIENT] Response status: 200
  ```

* **Backend terminal:**

  ```text
  [BACKEND] Telemetry: { status: 'cleaning', battery: 82 }
  ```

If this shows up, your **telemetry path is working** 

---

## 8. Quick Troubleshooting

* **`curl/curl.h: No such file or directory`**
  → Install dev package:

  ```bash
  sudo apt install -y libcurl4-openssl-dev
  ```

* **`microhttpd.h: No such file or directory`**
  → Install:

  ```bash
  sudo apt install -y libmicrohttpd-dev
  ```

* **`undefined reference to mosquitto_*`**
  → Install:

  ```bash
  sudo apt install -y libmosquitto-dev
  ```

* **Gateway can’t connect to MQTT** (`Connection refused`)
  → Check Mosquitto status:

  ```bash
  sudo systemctl status mosquitto
  ```

* **Backend says “connection refused” when hitting gateway**
  → Make sure `./c_gateway` is running and `GATEWAY_HTTP_PORT` in `config.h` matches `GATEWAY_URL` in `.env` (port 8000).

---

## 9. What This Proves

Once both tests pass, it demonstrates:

1. **Backend → Gateway → MQTT**

   * HTTP `POST /api/start` → gateway → publishes MQTT command to `robot/command`.

2. **MQTT → Gateway → Backend**

   * `mosquitto_pub` on `robot/telemetry` → gateway → HTTP `POST /api/telemetry` → backend.
