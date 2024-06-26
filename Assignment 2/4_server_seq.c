#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h> 

#define PORT 8085
#define MAX_CONN 10
#define MAX_FILENAME_LEN 256
#define MAX_BUFFER_SIZE 1024
#define IP "127.0.0.1"
#define RECEIVED_FILES_FOLDER "received_files"

void handle_connection(int client_socket)
{
    char filename[MAX_FILENAME_LEN];
    ssize_t bytes_received;

    DIR *dir;

    if ((dir = opendir(RECEIVED_FILES_FOLDER)) == NULL)
    {
        // mkdir(RECEIVED_FILES_FOLDER, 0777);
        if (mkdir(RECEIVED_FILES_FOLDER, 0777) == -1)
        {
            perror("Failed to open directory");
            close(client_socket);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        closedir(dir);
    }

    // Receive filename
    if ((bytes_received = recv(client_socket, filename, MAX_FILENAME_LEN, 0)) == -1)
    {
        perror("Receive failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    filename[bytes_received] = '\0';
    printf("Received file: %s\n", filename);

    char filePath[MAX_BUFFER_SIZE];
    snprintf(filePath, MAX_BUFFER_SIZE, "%s/%s", RECEIVED_FILES_FOLDER, filename);

    printf("File path: %s\n", filePath);

    // Open file
    FILE *file = fopen(filePath, "wb");
    if (!file)
    {
        perror("File open failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Receive file data and write to file
    ssize_t bytes_read;
    while (1)
    {
        char buffer[MAX_BUFFER_SIZE];
        bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
        buffer[bytes_read] = '\0';

        if (bytes_read == 0)
        {
            break;
        }

        if (bytes_read < 0)
        {
            perror("Receive failed");
            close(client_socket);
            fclose(file);
            exit(EXIT_FAILURE);
        }

        // printf("Received bytes: %ld\n", bytes_read);
        // printf("Received: %s\n", buffer);

        if (fwrite(buffer, 1, bytes_read, file) != bytes_read)
        {
            perror("File write failed");
            close(client_socket);
            fclose(file);
            exit(EXIT_FAILURE);
        }
    }

    // Close file and connection
    fclose(file);
    close(client_socket);
    printf("File received successfully: %s\n", filename);

    exit(EXIT_SUCCESS);
}

int main()
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Create server socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set server address parameters
    server_addr.sin_family = AF_INET;
    // server_addr.sin_addr.s_addr = INADDR_ANY;
    if (inet_pton(AF_INET, IP, &(server_addr.sin_addr)) <= 0)
    {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }
    server_addr.sin_port = htons(PORT);

    // Bind socket to address
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_socket, MAX_CONN) == -1)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept connections and handle them in separate processes
    while (1)
    {
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len)) == -1)
        {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        printf("Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        pid_t pid = fork();
        if (pid < 0)
        {
            perror("Error forking");
            close(client_socket);
            continue;
        }
        else if (pid == 0)
        {
            // Child process
            close(server_socket);
            handle_connection(client_socket);
            exit(EXIT_SUCCESS);
        }
        else
        {
            // Parent process
            close(client_socket);
        }
    }

    close(server_socket);
    return 0;
}