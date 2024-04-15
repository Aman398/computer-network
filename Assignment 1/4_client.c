#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <arpa/inet.h>
#include <dirent.h> 
#include <unistd.h>    // close() 

#define SERVER_IP "10.10.73.253"
#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

void send_folder(int client_socket, const char *folder_path, struct sockaddr_in server_addr) {

    struct dirent *entry;
    DIR *dir = opendir(folder_path);

    if (dir == NULL) {
        perror("Error opening folder");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char file_path[BUFFER_SIZE];
            sprintf(file_path, "%s/%s", folder_path, entry->d_name);

            // Send file name
            if (sendto(client_socket, entry->d_name, strlen(entry->d_name), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
                perror("Error sending data");
                exit(EXIT_FAILURE);
            }

            FILE *file = fopen(file_path, "rb");
            if (file == NULL) {
                perror("Error opening file");
                exit(EXIT_FAILURE);
            }

            while (1) {
                char buffer[BUFFER_SIZE];
                size_t bytes_read = fread(buffer, 1, BUFFER_SIZE, file);

                if (bytes_read == 0) {
                    // End of file
                    break;
                }

                if (sendto(client_socket, buffer, bytes_read, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
                    perror("Error sending data");
                    exit(EXIT_FAILURE);
                }
            }

            fclose(file);
            // Signal the end of the file
            sendto(client_socket, "", 0, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        }
    }

    closedir(dir);
    // Signal the end of the folder
    sendto(client_socket, "", 0, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
}

int main() {
    int client_socket;
    struct sockaddr_in server_addr;

    // Create socket
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(SERVER_PORT);

    const char *folder_path = "Transfer";  

    send_folder(client_socket, folder_path, server_addr);

    close(client_socket);

    printf("Folder sent to %s:%d\n", SERVER_IP, SERVER_PORT);

    return 0;
}
