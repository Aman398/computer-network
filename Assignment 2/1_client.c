#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h> 
#include <sys/socket.h>
#include <netinet/in.h>  
#include <arpa/inet.h>
 
#define PORT 8081
#define BUFFER_SIZE 1024

int main() {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    char expression[BUFFER_SIZE];
    printf("Enter postfix expression (e.g., 5 3 +): ");
    fgets(expression, BUFFER_SIZE, stdin);
    printf("Sending expression: %s", expression);
    send(sock, expression, strlen(expression), 0);
    if ((valread = read(sock, buffer, BUFFER_SIZE)) == 0) {
        printf("Server disconnected\n");
    }
    printf("Result: %s\n", buffer);
    memset(buffer, 0, sizeof(buffer));
    close(sock);
    return 0;
}