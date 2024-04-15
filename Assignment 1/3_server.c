#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>

struct sockaddr_in server, remote;
socklen_t remote_len = sizeof(remote);

int main()
{
    int port;
    printf("PORT: ");
    scanf("%d", &port);

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

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("error :%s", strerror(errno));
    }

    while (1)
    {
        /*receiving file name*/
        char file_name[1024];
        int received_file_name = recvfrom(sock, file_name, sizeof(file_name), 0, (struct sockaddr *)&remote, &remote_len);
        if (received_file_name == -1)
        {
            perror("error receiving");
            continue;
        }
        file_name[received_file_name] = '\0';

        FILE *file = fopen("hello.txt", "wb");
        if (file == 0)
        {
            perror("erro opening file");
            exit(EXIT_FAILURE);
        }

        /*receving data in chunks*/
        while (1)
        {
            char data_chunk[1024];
            int received_data = recvfrom(sock, data_chunk, sizeof(data_chunk), 0, (struct sockaddr *)&remote, &remote_len);
            if (received_data < 0)
            {
                perror("error receiving data");
                break;
            }
            if (received_data == 0)
            {
                break;
            }
            fwrite(data_chunk, 1, received_data, file);
        }
        fwrite("", 1, 0, file);
        fclose(file);
    }
    return 0;
}
