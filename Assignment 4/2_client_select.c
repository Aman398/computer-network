#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#define PORT 8080
#define MAX_BUFFER_SIZE 1024
// #define N 3 // Sender Window Size
#define IP "127.0.0.1"
#define TIMEOUT_SEC 3  // Timeout in seconds
#define TIMEOUT_USEC 0 // Timeout in microseconds
// #define N_lost 5       // Every N_lost Data frame will be lost
#define P 1.0                 // Probability with which Data frame will be lost
#define FILENAME "sample.txt" // Name of the file to be sent

typedef struct packet
{
    char data[1024];
} Packet;

typedef struct frame
{
    int frame_kind; // Acknowledgement Frame:0, Data Frame:1
    int sq_no;
    Packet packet;
} Frame;

int func(int N)
{
    srand(time(NULL));

    int sockfd;
    struct sockaddr_in serverAddress;
    char buffer[MAX_BUFFER_SIZE];
    socklen_t server_addr_size;

    FILE *file;
    ssize_t bytes_read;

    file = fopen(FILENAME, "rb");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    int start = 0;
    int next = start;
    int count = 0;
    Frame frame_recv;
    Frame frames[N];

    for (int i = 0; i < N; i++)
    {
        frames[i].packet.data[0] = '\0';
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddress, '\0', sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    // serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (inet_pton(AF_INET, IP, &(serverAddress.sin_addr)) <= 0)
    {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    server_addr_size = sizeof(serverAddress);

    fseek(file, 0, SEEK_END);
    int total_frames = (int)((double)ftell(file) / (double)MAX_BUFFER_SIZE) + ftell(file) % MAX_BUFFER_SIZE ? 2 : 1;
    fseek(file, 0, SEEK_SET);

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    while (1)
    {
        for (int i = next; i < start + N && start < total_frames; i++)
        {
            if (frames[i % N].packet.data[0] != '\0')
            {
                // printf("\n[+]Resending Frame %d\n", i);
            }
            else
            {
                frames[i % N].frame_kind = 1;
                frames[i % N].sq_no = i;

                bytes_read = fread(buffer, 1, MAX_BUFFER_SIZE, file);

                if (bytes_read == 0)
                {
                    // printf("\n[+]File Read Complete\n");
                    strcpy(frames[i % N].packet.data, "EOF");
                }
                else if (bytes_read < 0)
                {
                    perror("Error reading file");
                    exit(EXIT_FAILURE);
                }
                else
                    strcpy(frames[i % N].packet.data, buffer);

                // printf("[+]Sending Frame %d\n", i);
            }

            double random_number = (double)rand() / RAND_MAX;

            // if ((count + 1) % N_lost)
            if (random_number <= P)
            {
                sendto(sockfd, &frames[i % N], sizeof(Frame), 0, (struct sockaddr *)&serverAddress, server_addr_size);
                // printf("[+]Frame %d Sent\n", i);
            }
            count++;
        }

        next = start + N;

        while (1)
        {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(sockfd, &readfds);

            struct timeval timeout;
            timeout.tv_sec = TIMEOUT_SEC;
            timeout.tv_usec = TIMEOUT_USEC;

            int select_result = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

            if (select_result == -1)
            {
                perror("\nSelect Failed");
                exit(EXIT_FAILURE);
            }
            else if (select_result == 0)
            {
                // printf("\n[-]Timeout: Ack %d Not Received\n", start);
                next = start;
                break;
            }
            else
            {
                if (FD_ISSET(sockfd, &readfds))
                {
                    int recv_size = recvfrom(sockfd, &frame_recv, sizeof(frame_recv), 0, (struct sockaddr *)&serverAddress, &server_addr_size);

                    if (recv_size < 0)
                    {
                        perror("\nRecvfrom Failed");
                        exit(EXIT_FAILURE);
                    }
                    else if (recv_size > 0)
                    {
                        if (frame_recv.frame_kind == 0)
                        {
                            // printf("\n[+]Ack %d Received\n", frame_recv.sq_no);

                            if (frame_recv.sq_no >= start)
                            {
                                for (int j = start; j <= frame_recv.sq_no; j++)
                                {
                                    frames[j % N].packet.data[0] = '\0';
                                }

                                start = frame_recv.sq_no + 1;
                                break;
                            }
                            else
                            {
                                next = start;
                            }
                        }
                        else if (frame_recv.frame_kind == 1)
                        {
                            // printf("\n[-]Why is Server sending Data Frame???\n");
                        }
                        else
                        {
                            // printf("\n[-]Invalid Frame Received\n");
                        }
                    }
                    else
                    {
                        // printf("\n[-]Connection Closed\n");
                        break;
                    }
                }
            }
        }

        if (start >= total_frames)
        {
            break;
        }

        // printf("\n");
    }

    fclose(file);
    close(sockfd);

    gettimeofday(&end_time, NULL);

    long seconds = end_time.tv_sec - start_time.tv_sec;
    long microseconds = end_time.tv_usec - start_time.tv_usec;
    if (microseconds < 0)
    {
        seconds--;
        microseconds += 1000000;
    }
    // //printf("Total time taken: %ld seconds and %ld microseconds\n", seconds, microseconds);
    double final_time = seconds * 1000000.0 + microseconds;
    printf("%d,%lf\n", N, final_time);
    return 0;
}

int main()
{
    int Ns[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    for (int i = 0; i < sizeof(Ns) / sizeof(int); i++)
    {
        // ////printf("\n[+]N = %lf\n", Ns[i]);
        func(Ns[i]);
        // ////printf("\n\n");
    }
}