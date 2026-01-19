# Simple C++ HTTP Server (POSIX)

A tiny, blocking HTTP/1.1 server written in modern C++ on top of POSIX sockets (`getaddrinfo`, `socket`, `bind`, `listen`, `accept`). It accepts a connection, reads the request, and replies with a static HTML page.

This is intended as a learning / starter project for networking fundamentals.

## What it does

- Resolves bind addresses with `getaddrinfo()` (IPv4/IPv6).
- Binds a listening socket (default: all interfaces) and starts `listen()`.
- Accepts one connection at a time (`accept()`), reads the request, prints it, and sends back an HTTP response.
- Uses RAII for the `addrinfo` linked list via `std::unique_ptr` + `freeaddrinfo`.

## Requirements

- macOS or Linux (POSIX)
- C++20 compiler (this project uses `std::format`)
  - macOS: Apple Clang with a recent Xcode / Command Line Tools
  - Linux: GCC 13+ or Clang 16+ typically works

## Build

If you don’t have a build system yet, a single-file build looks like:

```bash
# Example (adjust paths/files to your project)
clang++ -std=c++20 -O0 -g \
  -Wall -Wextra -pedantic \
  src/main.cpp lib/network/Server.cpp \
  -o server
```

If you’re using CMake already, build as normal:

```bash
cmake -S . -B build
cmake --build build
```

## Run

Typical flow in `main()` is:

1. Create a `Server`
2. Call `startListen()`

Example:

```cpp
// main.cpp
#include "Server.h"

int main() {
    Server s(nullptr, "8080"); // bind all interfaces on port 8080
    s.startListen();
}
```

### Bind address options

- Local machine only:
  - IPv4 loopback: `127.0.0.1`
  - IPv6 loopback: `::1`

- All local interfaces:
  - Pass `nullptr` as IP with `AI_PASSIVE` (recommended)
  - Or bind explicitly to `0.0.0.0` (IPv4) / `::` (IPv6)

## Test with curl

### GET

```bash
curl -v http://localhost:8080/
```

### POST with JSON

```bash
curl -v -X POST http://localhost:8080/ \
  -H "Content-Type: application/json" \
  -d '{"msg":"hello","price":101,"qty":5}'
```

### Force IPv4 / IPv6

```bash
curl -4 -v http://127.0.0.1:8080/
curl -6 -v http://[::1]:8080/
```

## Customise the response

The response is built in `buildResponse()` as a simple HTTP message containing an HTML body. To serve different content:

- Replace the HTML string.
- Update the `Content-Type` / `Content-Length` accordingly.

## Notes / gotchas

- **TCP is a stream**: a single `read()` isn’t guaranteed to contain the full request. This demo reads up to `BUFFER_SIZE - 1` and prints what arrived.
- **Error handling**: if `bind()` fails for one candidate address, the server tries the next entry in the `getaddrinfo()` list.
- On macOS you may sometimes see system log lines such as `si_destination_compare ... send failed`. These are OS-internal logs that can appear during address ordering and are usually harmless.

## Limitations

- Blocking, single-threaded, one connection at a time.
- Minimal HTTP parsing (no routing, no chunked encoding, etc.).
- Intended for local development / learning, not production.
