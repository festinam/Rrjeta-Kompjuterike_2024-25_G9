# Server-Client File Management System

This project implements a server-client architecture for file management, where clients can interact with a server to perform file operations based on their access privileges. It is built using **socket programming** with **TCP** for communication.

## Overview

- **Server**: Listens for client connections, handles authentication, and enforces user privileges. Admin users can perform read, write, and execute commands on files, while read-only users can only read files.
- **Client**: Connects to the server, authenticates the user, and sends commands (list, read, write, execute) according to their privileges.

## Features

### Server:
1. Listens on a configurable IP address and port using **TCP sockets**.
2. Supports multiple clients using separate threads.
3. Differentiates access levels: admin users have full access, while read-only users can only read and list files.
4. Implements authentication with basic username/password validation.
5. Uses Winsock for socket communication and the C++ `<filesystem>` library for file operations.

### Client:
1. Connects to the server using the correct IP address and port via **TCP**.
2. Admin users can execute `read`, `write`, `list`, and `execute` commands.
3. Read-only users can only `read` files.
4. Supports command-line text-based input for interaction with the server.

## Requirements

- C++ with Winsock library (`Ws2_32.lib`)
- The `<filesystem>` library (C++17+)
- A Windows-based environment for running the application
- **TCP socket programming** for client-server communication
