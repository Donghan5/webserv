# 🌐 Webserv - High-Performance HTTP/1.1 Server

A robust, Nginx-inspired HTTP/1.1 web server built from scratch in **C++98**. This project focuses on low-level network programming, asynchronous I/O multiplexing, and strict adherence to the HTTP protocol.

## ✨ Key Features

* **Asynchronous Event Loop**: Utilizes `epoll` for efficient I/O multiplexing, allowing the server to handle multiple simultaneous client connections without the overhead of multi-threading.
* **HTTP/1.1 Compliant**: Supports core methods including `GET`, `POST`, and `DELETE`.
* **Dynamic Content via CGI**: Fully functional Common Gateway Interface (CGI) supporting multiple interpreters such as Python3, PHP, Perl, and Bash.
* **Flexible Configuration**: Features a custom Nginx-style configuration parser that manages virtual hosts, error pages, client body limits, and directory listing (autoindex).
* **Fault Tolerance**: Implements graceful shutdown using signal handlers (SIGINT, SIGQUIT) and robust memory management in a C++98 environment.

## 🛠 Tech Stack

* **Language**: C++98 (Standard compliant)
* **Core Concepts**: Socket Programming, TCP/IP, Non-blocking I/O, IPC (Pipes/Forking)
* **System Calls**: `epoll`, `socket`, `bind`, `listen`, `accept`, `fork`, `execve`, `pipe`, `waitpid`
* **Build System**: Makefile

## 🚀 Getting Started

### Prerequisites
* Linux environment (required for `epoll` support)
* C++ Compiler (c++)

### Compilation
To build the executable, run:
```bash
make

```

Other available commands include `make clean`, `make fclean`, and `make re`.

### Running the Server

The server requires a configuration file to start:

```bash
./webserv [path_to_config_file]

```

Example: `./webserv config/basic.conf`

## 🏗 Architecture Overview

* **`PollServer`**: The heart of the server that manages the `epoll` instance and dispatches events to the `RequestsManager`.
* **`CgiHandler`**: Manages external script execution using non-blocking pipes and handles process timeouts to prevent hanging resources.
* **`Parser`**: A modular component that builds the server's internal object model (`HttpConfig`, `ServerConfig`, `LocationConfig`) from the configuration file.

## 💡 Engineering Highlights

* **Non-blocking CGI Execution**: Unlike a basic implementation, this server uses `O_NONBLOCK` pipes and `WNOHANG` with `waitpid` to ensure that a slow CGI script never blocks the main server loop.
* **Protocol Integrity**: Successfully implemented complex HTTP features like custom error pages, MIME type mapping, and proper header management.
* **Memory Discipline**: Rigorous attention to manual memory management was applied to ensure the server can run long-term without leaks, using explicit destructor patterns (`_self_destruct()`).
