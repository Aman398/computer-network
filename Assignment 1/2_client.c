// now I want to make a client which will send message periodically to 5 servers with same IP addresses, after 2 seconds of interval.,
// that is it will send message to 5 servers in a round robin fashion.
// the port of all the servers, will be taken as input from the user, and then it will send message to all the servers in a round robin fashion.

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <time.h>

#define BUF_SIZE 30

void error_handling(char *message);

int main(int argc, char *argv[])
{
    int clnt_sock;
    char message[BUF_SIZE];
    int str_len;
    socklen_t serv_adr_sz;

    struct sockaddr_in serv_adr;

    if (argc != 6)
    {
        printf("Usage: %s <port1> <port2> <port3> <port4> <port5>\n", argv[0]);
        exit(1);
    }

    clnt_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (clnt_sock == -1)
        error_handling("UDP socket creation error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr("10.10.73.253");
    serv_adr.sin_port = htons(atoi(argv[1]));

    serv_adr_sz = sizeof(serv_adr);

    printf("Client is running...\n");

    while (1)
    {
        for (int i = 1; i < 6; i++)
        {
            serv_adr.sin_port = htons(atoi(argv[i]));
            sendto(clnt_sock, "Hello", BUF_SIZE, 0, (struct sockaddr *)&serv_adr, serv_adr_sz);
            // printf("Message sent to server %d\n", i);
            str_len = recvfrom(clnt_sock, message, BUF_SIZE, 0, (struct sockaddr *)&serv_adr, &serv_adr_sz);
            message[str_len] = 0;
            printf("Message from server: %s\n", message);
            sleep(2);
        }
    }

    close(clnt_sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
}