#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <arpa/inet.h> 

#define PORT 8081
#define BUFFER_SIZE 1024

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    // Receive the file name from the client
    memset(buffer, 0, sizeof(buffer));
    bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    buffer[sizeof(buffer)] = '\0';  
    if (bytes_received < 0) {
        perror("Error receiving file name");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    printf("Received file name: %s\n", buffer);

    // Open a file for writing
    FILE *file = fopen("recv", "wb");
    if (!file) {
        perror("Error opening file");
        close(client_socket); 
        exit(EXIT_FAILURE);
    }

    while(1){
        char buffer2[BUFFER_SIZE];
        memset(buffer2, 0, sizeof(buffer2));
        bytes_received = recv(client_socket, buffer2, sizeof(buffer2), 0);
        if(bytes_received < 0){
            perror("Error receiving file");
            break;
        }
        if(bytes_received == 0){
            break;
        }
        fwrite(buffer2, 1, bytes_received, file);
    }
    fclose(file);
    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);
 
    // Bind socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 10) == -1) {
        perror("Error listening for connections");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // Accept a connection from a client
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);
        if (client_socket == -1) {
            perror("Error accepting connection");
            close(server_socket);
            exit(EXIT_FAILURE);
        }
        printf("Accepted connection from %s: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        int pid = fork();
        if (pid == 0) {
            close(server_socket);  // Child process doesn't need the listening socket
            handle_client(client_socket);
            exit(EXIT_SUCCESS);
        } else if (pid == -1) {
            perror("Error forking");
            close(client_socket);
        } 
        else {
            close(client_socket);  // Parent process doesn't need the connected socket
        }
    }
    return 0;
}