# C++ Multi-Threaded HTTP Server

A high-performance, multi-threaded HTTP server engineered from scratch in C++. This project was developed to gain a deep understanding of Linux system-level programming, concurrency models, and network protocol implementation.

## Technical Architecture & Features

- **Language:** C++
- **Networking:** POSIX Sockets (`<sys/socket.h>`)
- **Concurrency Model:** Custom fixed-size thread pool utilizing task queues, POSIX condition variables, and mutex locks to eliminate thread creation overhead and handle massive request loads without spawning new threads per connection.
- **Design Philosophy:** Optimized for high throughput and low latency, utilizing `SO_REUSEADDR` and `SOMAXCONN` socket tuning. Parses HTTP keep-alive connections to reuse sockets.
- **API Endpoint:** Serves a dynamic `/sum?a=<val>&b=<val>` endpoint that parses HTTP GET requests and query parameters to compute a response.

## Performance Benchmark

Tested on a Linux VM using `wrk`.

**Command:** `wrk -t6 -c400 -d10s "http://127.0.0.1:8080/sum?a=123&b=456"` (with server running 6 threads)

**Results:**
* **Throughput:** ~82,550 Requests/sec
* **Avg Latency:** ~60.12µs (microseconds)
* **Stability:** Zero connection crashes under heavy concurrent load (400 connections).

## Workflow

1.  **Listener:** The main thread binds to a socket and listens for incoming connections.
2.  **Dispatch:** Upon `accept()`, the client file descriptor is enqueued into the Thread Pool.
3.  **Worker:** An available thread from the pool retrieves the socket, parses the raw HTTP text buffers, extracts the query parameters, performs the computation, and writes back the HTTP response format.

## Quick Start

### 1. Compile
Build the server using the provided `Makefile`:
```bash
make
```

### 2. Start the Server
Run the server executable. You can optionally pass the number of worker threads (default is 4):
```bash
./my_server 8
```

### 3. Test the API
Use `curl` to query the endpoint:
```bash
curl "http://localhost:8080/sum?a=10&b=25"
```
*(Should return a quick HTML response containing `35`)*