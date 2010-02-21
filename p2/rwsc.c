/*Reliable Web Server Client
	David Audet - V00150102
*/

#include<stdio.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<time.h>
#include<string.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<ctype.h>
#include"rdp.h"

int main(int argc, char** argv){
	int sockfd;
	struct sockaddr_in sendaddr;
	struct sockaddr_in recvaddr;
	

	int optval = 1;

	if( argc != 6){
		printf("Incorrect number of parameters!\nUsage: ./rwsc client_ip client_port server_ip server_port URL\n");
		return -1;
	}

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("socket");
		return -1;
	}
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	sendaddr.sin_family = AF_INET;
	sendaddr.sin_port = htons(atoi(argv[2]));
	sendaddr.sin_addr.s_addr = inet_addr(argv[1]);

	if(bind(sockfd, (struct sockaddr*)&sendaddr, sizeof(sendaddr)) == -1){
		perror("bind");
		return -1;
	}
	
	recvaddr.sin_family = AF_INET;
	recvaddr.sin_port = htons(atoi(argv[4]));
	recvaddr.sin_addr.s_addr = inet_addr(argv[3]);
	//struct to hold the receiver's address
//delete later
	rdp_connect(sockfd, argv[3], argv[4]);
	char* storage = malloc(sizeof(char)*990);
//	rdp_recv(sockfd, storage, sizeof(storage), &recvaddr); 
	char* request = "GET / HTTP/1.0\r\n\r\n";
	rdp_send(sockfd, request, strlen(request), &recvaddr);
	rdp_recv(sockfd, storage, strlen(storage), &recvaddr); 
	printf("%s\n", storage);
	rdp_recv(sockfd, storage, strlen(storage), &recvaddr);
	printf("%s\n", storage);
	close(sockfd);
	return 0;
}
