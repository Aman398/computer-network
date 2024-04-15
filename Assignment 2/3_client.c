#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <arpa/inet.h> 
 
#define PORT 8081
#define BUFFER_SIZE 1024

void send_file(const char *file_name, int socket) {
    FILE *file;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read, bytes_sent;

    // Send file name to server 
    printf("Sending file %s\n", file_name);
    bytes_sent = send(socket, file_name, strlen(file_name), 0);
    if (bytes_sent < 0) {
        perror("Error sending file name");
        close(socket);
        exit(EXIT_FAILURE);
    }
    sleep(1); // Ensure file name is sent before file data

    // Open file
    file = fopen(file_name, "rb");
    if (file == NULL) {
        perror("Error opening file");
        close(socket);
        exit(EXIT_FAILURE);
    }

    // Send file data to server
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        bytes_sent = send(socket, buffer, bytes_read, 0);
        if (bytes_sent < 0) {
            perror("Error sending file data");
            fclose(file);
            close(socket);
            exit(EXIT_FAILURE);
        }
    }

    fclose(file);
    printf("File %s sent successfully\n", file_name);
}

int main() {
    int client_socket;
    struct sockaddr_in server_addr;

    // Create client socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Error creating client socket");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Send file
    char file_name[1024];
    printf("Enter file name: ");
    scanf("%s", file_name);
    send_file(file_name, client_socket);

    // Close client socket
    close(client_socket);

    return 0;
}
