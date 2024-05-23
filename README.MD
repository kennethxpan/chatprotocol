# Chat Protocol Implementation Using QUIC

## Overview

This project implements a simple chat protocol using the QUIC protocol for transport. The implementation includes state management, authentication, instant messaging, and dynamic server discovery. The server can handle multiple clients concurrently.

## Features

- **State Management**: Implements a stateful protocol with states: DISCONNECTED, CONNECTED, AUTHENTICATED, and CHATTING.
- **Instant Messaging**: Supports sending and receiving text messages.
- **Dynamic Server Discovery**: Uses multicast to discover the server.
- **Concurrent Server**: Handles multiple clients simultaneously using threads.


## Requirements

- **C Compiler**: gcc
- **Libraries**: ngtcp2, pthreads

## Running the Application
Use "Make" command to make client and server files
"Make Clean" to undo make

### Server

1. **Start the server**
    ```sh
    ./server
    ```

    - The server will start listening for client connections and announce its presence using multicast.

### Client

1. **Start the client**
    ```sh
    ./client
    ```

    - The client will automatically discover the server using multicast and connect to it.

### Usage

- **Sending Messages**: Type your message and press Enter to send.
- **Receiving Messages**: Messages from other clients will appear automatically.
- **Exit**: Type `exit` and press Enter to disconnect.

## Dynamic Server Discovery

- The client uses multicast to discover the server. The server periodically announces its presence on a multicast group, and the client listens for these announcements to find the server.
