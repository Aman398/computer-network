// Common Header Files and Macros
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> 
#include <string.h>
#include <unistd.h> 
#include <arpa/inet.h> 
 
#define PORT 8081
#define BUFFER_SIZE 1024

// Stack type
struct Stack {
	int top;
	unsigned capacity;
	int* array;
};

// Stack Operations
struct Stack* createStack(unsigned capacity) {
	struct Stack* stack = (struct Stack*)malloc(sizeof(struct Stack));

	if (!stack)
		return NULL;

	stack->top = -1;
	stack->capacity = capacity;
	stack->array = (int*)malloc(stack->capacity * sizeof(int));

	if (!stack->array)
		return NULL;

	return stack;
}

int isEmpty(struct Stack* stack) {
	return stack->top == -1;
}

int peek(struct Stack* stack) {
	return stack->array[stack->top];
}

int pop(struct Stack* stack) {
	if (!isEmpty(stack))
		return stack->array[stack->top--];

    printf("Stack is empty\n");
	return 0;
}

void push(struct Stack* stack, int op) {
	stack->array[++stack->top] = op;
}

int evaluatePostfix(char* exp) {
	struct Stack* stack = createStack(strlen(exp)); 
	int i;

	if (!stack)
		return -1;

	for (i = 0; i < strlen(exp)-1; ++i) {
        if(exp[i] == '\0') 
            continue;

		// if the character is blank space then continue
		if (exp[i] == ' ')
			continue;

		// If the scanned character is an
		// operand (number here),extract the full number
		// Push it to the stack.
		else if (isdigit(exp[i])) {
			int num = 0;

			// extract full number
			while (isdigit(exp[i])) {
				num = num * 10 + (int)(exp[i] - '0');
				i++;
			}
			i--;

			// push the element in the stack
			push(stack, num);
		}
		// If the scanned character is an operator, pop two
		// elements from stack apply the operator
		else {
			int val1 = pop(stack);
			int val2 = pop(stack);

			switch (exp[i]) {
			case '+':
				push(stack, val2 + val1);
				break;
			case '-':
				push(stack, val2 - val1);
				break;
			case '*':
				push(stack, val2 * val1);
				break;
			case '/':
				push(stack, val2 / val1);
				break;
			}
		}
	}
    return pop(stack);
}

void process_and_respond(int socket, char* buffer, struct sockaddr_in* client_address, socklen_t client_address_len) {
    printf("Received from %s:%d: %s",inet_ntoa(client_address->sin_addr), ntohs(client_address->sin_port), buffer);
    buffer[strlen(buffer)] = '\0';
	int answer = evaluatePostfix(buffer);
    printf("Answer: %d\n", answer); 
    char result[100];   
    snprintf(result, 100, "%d", answer);
    printf("Result: %s\n\n", result);
    // Send a response back to the client
    sendto(socket, result, strlen(result), 0, (struct sockaddr*)client_address, client_address_len);
} 

int main() { 
    int tcp_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);

    char buffer[BUFFER_SIZE];

    // Create TCP socket
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket == -1) {
        perror("TCP Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(tcp_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("TCP Bind failed");
        close(tcp_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(tcp_socket, 5) == -1) {  // Backlog set to 5 for simplicity
        perror("TCP Listen failed");
        close(tcp_socket);
        exit(EXIT_FAILURE);
    }

    printf("TCP Server listening on port %d...\n\n", PORT);

    while (1) {
        // Accept incoming connection
        client_socket = accept(tcp_socket, (struct sockaddr*)&client_address, &client_address_len);
        if (client_socket == -1) {
            perror("TCP Accept failed");
            continue;
        }
        printf("Client connected from IP: %s, Port: %d\n",
               inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        // Receive data from the client
        ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received == -1) {
            perror("TCP Receive failed");
            close(client_socket);
            continue;
        }

        buffer[bytes_received] = '\0';

        // Call common function for processing and responding
        process_and_respond(client_socket, buffer, &client_address, client_address_len);

        // Close the client socket
        close(client_socket);
    }

    close(tcp_socket);
    return 0;
}