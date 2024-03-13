#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define NUM_CLIENTS 260  // Number of clients to simulate
#define SERVER_PORT 8000  // Server port numbers

void* create_client(void* arg) {
    int client_socket;
    struct sockaddr_in server_addr;

    // Create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Connect to server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Client connected\n");

    // Keep the connection open
    while (1) {
        sleep(1);
    }

    close(client_socket);
    return NULL;
}

int main() {
    pthread_t threads[NUM_CLIENTS];

    // Create clients
    for (int i = 0; i < NUM_CLIENTS; i++) {
        if (pthread_create(&threads[i], NULL, create_client, NULL) != 0) {
            perror("Failed to create thread");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all clients to finish
    for (int i = 0; i < NUM_CLIENTS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}