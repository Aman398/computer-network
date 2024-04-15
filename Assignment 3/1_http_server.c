#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081
#define BUFFER_SIZE 1024 

void handle_request(int newsock) {
    char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: keep-alive\r\nKeep-Alive: timeout=5, max=100\r\n\r\n";
    FILE *file = fopen("index.html", "r");

    if (file == NULL) {
        perror("Error opening index.html");
        return;
    }

    send(newsock, response, strlen(response), 0);

    char buffer[BUFFER_SIZE];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(newsock, buffer, bytesRead, 0);
    }
    

    fclose(file);
    int bzise = recv(newsock,buffer,sizeof(buffer),0);
    close(newsock);
    
}

int main() {
    int server_socket, newsock;
    struct sockaddr_in server_address, client_address;
    socklen_t address_len = sizeof(server_address);

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
 


    // Configure server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 10) == -1) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // Accept a connection
        // printf("Waiting for connection request\n");
        if ((newsock = accept(server_socket, (struct sockaddr *)&client_address, &address_len)) == -1) {
                perror("Accept failed");
                continue;
            }
        printf("Connection accepted from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));    
        int pid = fork();
        if(pid==0)
        {
            // Handle the HTTP request
            handle_request(newsock);
            

        }
        if(pid<0){perror("fork");}
        if(pid>0)
        {
           close(newsock);
        }
    }

    close(server_socket);

    return 0;
}
