#ifndef QUIC_LIB_H
#define QUIC_LIB_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ngtcp2/ngtcp2.h> // Include ngtcp2 library for QUIC protocol support
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// Define the server port number
#define SERVER_PORT 22222

// Define the structure for Protocol Data Unit (PDU)
typedef struct {
    uint8_t messageType;      // Type of the message
    uint32_t messageLength;   // Length of the message
    uint64_t timestamp;       // Timestamp of the message
    char senderID[16];        // ID of the sender
    char payload[256];        // Payload of the message
    uint32_t checksum;        // Checksum for error detection
} PDU;

// Function to handle server connection
void handle_connect(int *sock, struct sockaddr_in *address);

// Function to handle server disconnection
void handle_disconnect(int sock);

// Function to handle authentication
void handle_authentication(const char* username, const char* password);

// Function to send a message
void send_message(int sock, const char* message);

// Function to receive a message
void receive_message(int sock);

// Function to parse PDU from raw data
void parse_pdu(const uint8_t* data, PDU* pdu);

// Function to serialize PDU to raw data
void serialize_pdu(const PDU* pdu, uint8_t* data);

// Function to encrypt a message
void encrypt_message(const char* message, char* encryptedMessage);

// Function to add a checksum to a PDU
void add_checksum(PDU* pdu);

// Function to handle errors
void handle_error(const char* errorMessage);

// Enum for representing different states of the protocol
typedef enum {
    STATE_DISCONNECTED,  // Disconnected state
    STATE_CONNECTED,     // Connected state
    STATE_AUTHENTICATED, // Authenticated state
    STATE_CHATTING       // Chatting state
} State;

// Global variable to hold the current state of the protocol
extern State currentState;

// Function to transition to a new state
void transition_state(State newState);

#endif // QUIC_LIB_H
