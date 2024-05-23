#include "quic_lib.h"
#include <pthread.h>
#include <arpa/inet.h>

#define MULTICAST_GROUP "239.255.255.250"
#define MULTICAST_PORT 1900

/**
 * Discovers the server using multicast.
 * 
 * @return The port number of the discovered server.
 */
int discover_server() {
    int sock;
    struct sockaddr_in addr;
    struct ip_mreq mreq;
    char message[256];
    int server_port = 0;

    // Create a socket for multicast
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Multicast socket creation failed");
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(MULTICAST_PORT);

    // Bind the socket to the multicast port
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("Multicast bind failed");
        exit(1);
    }

    mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    // Join the multicast group
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("Multicast setsockopt failed");
        exit(1);
    }

    // Listen for server announcements
    while (1) {
        if (recv(sock, message, sizeof(message), 0) < 0) {
            perror("Multicast recv failed");
            exit(1);
        }

        if (strncmp(message, "CHAT_SERVER_ANNOUNCEMENT", 24) == 0) {
            server_port = SERVER_PORT; // Assuming server port is known and constant
            break;
        }
    }

    // Close the multicast socket
    close(sock);
    return server_port;
}

/**
 * Receives messages from the server.
 * 
 * @param arg Pointer to the socket descriptor.
 * @return NULL
 */
void* receive_messages(void* arg) {
    int sock = *(int*)arg;
    while (1) {
        uint8_t data[sizeof(PDU)];
        int valread = read(sock, data, sizeof(data));

        if (valread > 0) {
            PDU pdu;
            parse_pdu(data, &pdu);
            printf("\33[2K\rMessage received from %s: %s\n", pdu.senderID, pdu.payload);
            printf("Enter message to send (or 'exit' to disconnect): ");
            fflush(stdout); // Flush the output to ensure prompt is displayed correctly
        } else {
            handle_error("Failed to receive message");
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    const char *server_ip = "127.0.0.1";
    int server_port = discover_server();

    if (server_port == 0) {
        fprintf(stderr, "Server discovery failed\n");
        exit(EXIT_FAILURE);
    }

    int sock = 0;
    struct sockaddr_in serv_addr;

    // Create a socket for the client
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        handle_error("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    // Convert IP address to binary form
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        handle_error("Invalid address/ Address not supported");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        handle_error("Connection Failed");
        return -1;
    }

    transition_state(STATE_CONNECTED);

    // Simulate authentication
    handle_authentication("user", "pass");

    // Simulate chat state
    transition_state(STATE_CHATTING);

    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_messages, &sock);

    char buffer[256];
    while (1) {
        printf("Enter message to send (or 'exit' to disconnect): ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0; // Trim newline character

        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        send_message(sock, buffer);
    }

    handle_disconnect(sock);
    return 0;
}
