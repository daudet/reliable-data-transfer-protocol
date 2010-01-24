/* 	sws.c - Simple Web Server
 *	CSc 361 January 29th, 2010
 *	David Audet - V00150102
 */

#include<arpa/inet.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<time.h>
#include<fcntl.h>
#include<sys/stat.h>

void printlog(int seqNo, char* clientAddr, int port, char* message, char* response, char* resource);

int main(int argc, char** argv){

	int serverSock, clientSock;
	int optval = 1;
	struct sockaddr_in client, server;
	struct stat stbuf;
	socklen_t length = sizeof(struct sockaddr);

	if(argc != 3){
		printf("Incorrect number of parameters!\nUsage: ./sws <port> <directory>\n");
		return -1;
	}
	if((serverSock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("Could not create the socket");
		return -1;
	}
	if(setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1){
		perror("Could not set socket options");
		return -1;
	}
	//populate the struct for the address information
	server.sin_family = AF_INET;
	//fill in the local ip address of the server automatically
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(atoi(argv[1]));

/* BIND THE SOCKET */
	if((bind(serverSock, (struct sockaddr*)&server, sizeof(server))) == -1){
		perror("Could not bind socket");
		return -1;
	}

	if((listen(serverSock, 5)) == -1){
		perror("Could not listen on socket");
		return -1;
	}
	
	//Check for valid port number
	if((atoi(argv[1]) < 1025) || (atoi(argv[1]) > 65535)){
		perror("Invalid port number for this server");
		return -1;
	}
	//Check for a valid serving directory
	if((stat(argv[2], &stbuf)) == -1){
		perror("Invalid directory to serve");
		return -1;
	}

	printf("sws is running on TCP port %d and serving %s\n", atoi(argv[1]), argv[2]);
	printf("press 'q' to quit ...\n");

	return 0;
}
