#include "quic_lib.h"

// Initialize the current state to DISCONNECTED
State currentState = STATE_DISCONNECTED;

/**
 * Parses a PDU from raw data.
 * 
 * @param data The raw data to parse.
 * @param pdu The PDU structure to fill.
 */
void parse_pdu(const uint8_t* data, PDU* pdu) {
    memcpy(&pdu->messageType, data, sizeof(pdu->messageType));
    memcpy(&pdu->messageLength, data + 1, sizeof(pdu->messageLength));
    memcpy(&pdu->timestamp, data + 5, sizeof(pdu->timestamp));
    memcpy(pdu->senderID, data + 13, sizeof(pdu->senderID));
    memcpy(pdu->payload, data + 29, pdu->messageLength - 33);
    memcpy(&pdu->checksum, data + pdu->messageLength - 4, sizeof(pdu->checksum));
}

/**
 * Serializes a PDU into raw data.
 * 
 * @param pdu The PDU structure to serialize.
 * @param data The buffer to hold the serialized data.
 */
void serialize_pdu(const PDU* pdu, uint8_t* data) {
    memcpy(data, &pdu->messageType, sizeof(pdu->messageType));
    memcpy(data + 1, &pdu->messageLength, sizeof(pdu->messageLength));
    memcpy(data + 5, &pdu->timestamp, sizeof(pdu->timestamp));
    memcpy(data + 13, pdu->senderID, sizeof(pdu->senderID));
    memcpy(data + 29, pdu->payload, pdu->messageLength - 33);
    memcpy(data + pdu->messageLength - 4, &pdu->checksum, sizeof(pdu->checksum));
}

/**
 * Encrypts a message. For simplicity, this function performs identity encryption.
 * 
 * @param message The original message.
 * @param encryptedMessage The buffer to hold the encrypted message.
 */
void encrypt_message(const char* message, char* encryptedMessage) {
    // Example encryption (identity function for simplicity)
    strcpy(encryptedMessage, message);
}

/**
 * Adds a checksum to a PDU for error detection.
 * 
 * @param pdu The PDU to which the checksum is added.
 */
void add_checksum(PDU* pdu) {
    uint32_t checksum = 0;
    uint8_t* data = (uint8_t*)pdu;
    for (uint32_t i = 0; i < pdu->messageLength - 4; ++i) {
        checksum += data[i];
    }
    pdu->checksum = checksum;
}

/**
 * Handles errors by printing an error message.
 * 
 * @param errorMessage The error message to print.
 */
void handle_error(const char* errorMessage) {
    printf("Error: %s\n", errorMessage);
}

/**
 * Transitions the state of the protocol.
 * 
 * @param newState The new state to transition to.
 */
void transition_state(State newState) {
    const char* stateNames[] = {
        "DISCONNECTED",
        "CONNECTED",
        "AUTHENTICATED",
        "CHATTING"
    };
    currentState = newState;
    printf("Transitioned to state: %s\n", stateNames[newState]);
}

/**
 * Handles server connection setup.
 * 
 * @param sock The socket descriptor.
 * @param address The socket address structure.
 */
void handle_connect(int *sock, struct sockaddr_in *address) {
    *sock = socket(AF_INET, SOCK_STREAM, 0);
    if (*sock == -1) {
        handle_error("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons(SERVER_PORT);

    if (bind(*sock, (struct sockaddr *)address, sizeof(*address)) < 0) {
        handle_error("Port Used");
        exit(EXIT_FAILURE);
    }

    if (listen(*sock, 3) < 0) {
        handle_error("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", SERVER_PORT);
}

/**
 * Handles server disconnection.
 * 
 * @param sock The socket descriptor.
 */
void handle_disconnect(int sock) {
    close(sock);
    printf("Disconnected\n");
}

/**
 * Handles authentication of a user.
 * 
 * @param username The username.
 * @param password The password.
 */
void handle_authentication(const char* username, const char* password) {
    if (strcmp(username, "user") == 0 && strcmp(password, "pass") == 0) {
        printf("Authentication successful\n");
        transition_state(STATE_AUTHENTICATED);
    } else {
        handle_error("Authentication failed");
        transition_state(STATE_DISCONNECTED);
    }
}

/**
 * Sends a message to the specified socket.
 * 
 * @param sock The socket descriptor.
 * @param message The message to send.
 */
void send_message(int sock, const char* message) {
    PDU pdu;
    pdu.messageType = 1;
    pdu.messageLength = sizeof(PDU);
    pdu.timestamp = time(NULL);
    strcpy(pdu.senderID, "server");
    strcpy(pdu.payload, message);
    add_checksum(&pdu);

    uint8_t data[sizeof(PDU)];
    serialize_pdu(&pdu, data);

    send(sock, data, sizeof(data), 0);
    printf("Message sent: %s\n", message);
}

/**
 * Receives a message from the specified socket.
 * 
 * @param sock The socket descriptor.
 */
void receive_message(int sock) {
    uint8_t data[sizeof(PDU)];
    int valread = read(sock, data, sizeof(data));

    if (valread > 0) {
        PDU pdu;
        parse_pdu(data, &pdu);
        printf("Message received from %s: %s\n", pdu.senderID, pdu.payload);
    } else {
        handle_error("Failed to receive message");
    }
}
