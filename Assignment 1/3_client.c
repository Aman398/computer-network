#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

struct sockaddr_in server, remote;
socklen_t remote_len = sizeof(remote);
char buffer[1024];

int main()
{
    int port = 8080;

    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("socket");
    }

    /*server addr struct*/
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(port);

    char *file_path = "check1.txt";
    FILE *file = fopen(file_path, "rb");

    /*sending file name*/
    if (sendto(sock, file_path, strlen(file_path), 0,
               (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror("Error sending file name");
        exit(EXIT_FAILURE);
    }

    /*sending data*/
    socklen_t server_len = sizeof(server);
    while (1)
    {
        char data_chunk[1024];
        int read_data;
        read_data = fread(data_chunk, 1, sizeof(data_chunk), file);
        if (read_data < 0)
        {
            perror("error in sending data");
            break;
        }
        if (read_data == 0)
        {
            break;
        }
        if (sendto(sock, data_chunk, read_data, 0,
                   (struct sockaddr *)&server, sizeof(server)) == -1)
        {
            perror("error sending data chunk");
        }
    }

    fclose(file);
    return 0;
}
