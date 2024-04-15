#include <stdio.h> 
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

typedef struct packet{
    char data[1024];
}Packet;

typedef struct frame{
    int frame_kind; //ACK:0, SEQ:1 FIN:2
    int sq_no;
    int ack;
    Packet packet;
}Frame;

int set_timeout(int sockfd, int sec){
    fd_set readfds;
    struct timeval timeout;

    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);

    timeout.tv_sec = sec;
    timeout.tv_usec = 0;

    return select(sockfd + 1 , &readfds, NULL, NULL, &timeout);
}

int main(int argc, char **argv[]){
    if (argc != 2){
		printf("Usage: %s <port>", argv[0]);
		exit(0);	
	}

	int port = atoi(argv[1]);
	int sockfd;
	struct sockaddr_in serverAddr;
	char buffer[1024];
	socklen_t addr_size;

	int frame_id = 0;
	Frame frame_send;
	Frame frame_recv;
	int ack_recv = 1, changed;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	 
	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	while(1){

		frame_send.sq_no = frame_id;
		frame_send.frame_kind = 1;
		frame_send.ack = 0;		
	
		printf("Enter Data: ");
		scanf("%s", buffer);
		strcpy(frame_send.packet.data, buffer);
		
        Start:
        sendto(sockfd, &frame_send, sizeof(Frame), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
		printf("[+]Frame Send\n");
		
        // let time out of 2 seconds.....
        Wait:
        changed = set_timeout(sockfd, 1);
        if(changed == 0){
            printf("Timeout, re-sending the same sequence data again\n");
            goto Start;
        }
        int addr_size = sizeof(serverAddr);
		int f_recv_size = recvfrom(sockfd, &frame_recv, sizeof(frame_recv), 0 ,(struct sockaddr*)&serverAddr, &addr_size);        
		
		if(f_recv_size > 0 && frame_recv.sq_no == 0 && frame_recv.ack == frame_id){
			printf("[+]Expected Ack Received\n");
            frame_id++;
		}
        else if(f_recv_size > 0 && frame_recv.sq_no == 0){
			printf("[-]Expected Ack Not Received, i.e., previous ACK \n");
			goto Wait;
		}	
        else if(f_recv_size > 0){
            printf("Why server is sending me data, I will send data, they should send me ACK only\n");
        }
        else{
            printf("No data received\n");
        }
	}
	close(sockfd);
	return 0;
}