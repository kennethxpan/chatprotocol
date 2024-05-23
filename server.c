#include "quic_lib.h"
#include <pthread.h>
#include <arpa/inet.h>

#define MULTICAST_GROUP "239.255.255.250"
#define MULTICAST_PORT 1900

// Function prototype for handle_client
void* handle_client(void* arg);

/**
 * Sends multicast announcements to help clients discover the server.
 * 
 * @param arg Unused parameter.
 * @return NULL
 */
void* multicast_announcement(void* arg) {
    int sock;
    struct sockaddr_in addr;
    char *message = "CHAT_SERVER_ANNOUNCEMENT";

    // Create a socket for multicast
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Multicast socket creation failed");
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP);
    addr.sin_port = htons(MULTICAST_PORT);

    // Periodically send multicast announcements
    while (1) {
        if (sendto(sock, message, strlen(message), 0, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
            perror("Multicast sendto failed");
            exit(1);
        }
        sleep(5); // Send announcement every 5 seconds
    }
}

/**
 * Receives messages from a client.
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

/**
 * Handles communication with a client.
 * 
 * @param arg Pointer to the socket descriptor.
 * @return NULL
 */
void* handle_client(void* arg) {
    int new_socket = *(int*)arg;
    free(arg);

    // Simulate authentication
    handle_authentication("user", "pass");

    // Simulate chat state
    transition_state(STATE_CHATTING);

    // Create a thread to receive messages from the client
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_messages, &new_socket);

    char buffer[256];
    while (1) {
        printf("Enter message to send (or 'exit' to disconnect): ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0; // Trim newline character

        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        send_message(new_socket, buffer);
    }

    handle_disconnect(new_socket);
    return NULL;
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Set up server connection
    handle_connect(&server_fd, &address);
    transition_state(STATE_CONNECTED);

    // Create a thread to send multicast announcements
    pthread_t multicast_thread;
    pthread_create(&multicast_thread, NULL, multicast_announcement, NULL);

    // Main loop to accept incoming client connections
    while (1) {
        int* new_socket = malloc(sizeof(int));
        if ((*new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            handle_error("Accept failed");
            free(new_socket);
            continue;
        }

        // Create a thread to handle the new client
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, new_socket);
        pthread_detach(thread_id);
    }

    handle_disconnect(server_fd);
    return 0;
}
