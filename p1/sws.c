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
#include<pthread.h>

struct sock_data{
	int socketfd;
	struct sockaddr_in* addrinfo;
};

void print_log(int seqNo, char* clientAddr, int port, char* message, char* response, char* resource);
void handle_connection(struct sock_data*);

void print_log(int seqNo, char* clientAddr, int port, char* message, char* response, char* resource){
	time_t time_val;
	struct tm* tmstring;
	char timestring[256];
	time(&time_val);
	tmstring = localtime(&time_val);
	strftime(timestring, sizeof(timestring), "%Y %b %d %T", tmstring);
	printf("Seq no. %d %s %s:%d %s;%s;\n%s\n", seqNo, timestring, clientAddr, port, message, response, resource);
	
}
void handle_connection(struct sock_data* client){
	
	int receivedMsgSize;
	char* request = malloc(sizeof(char)*512);
	char* next_request = malloc(sizeof(char)*512);
	char* response = malloc(sizeof(char)*512);
	char* resource = malloc(sizeof(char)*512);
	char* method = malloc(sizeof(char)*32);
	char* protocol = malloc(sizeof(char)*32);
	int n = 0;

	for(;;){
		
		if((receivedMsgSize = recv(client->socketfd, request, 512, 0)) < 0)
			perror("Error receiving message\n");
		request[strlen(request)-1] = '\0';
		do{	
			if((receivedMsgSize = recv(client->socketfd, next_request, 512, 0)) < 0)
			perror("Error receiving message\n");
		}
		while(strcmp(next_request, "\n") != 0);

		printf("%s\n", request);
		method = strtok(request, " ");
		resource = strtok(NULL, " ");
		protocol = strtok(NULL, " ");
		response = "HTTP/1.0 200 OK";
	
		print_log(++n, inet_ntoa(client->addrinfo->sin_addr), ntohs(client->addrinfo->sin_port), request, response, resource);
		send(client->socketfd, response, strlen(response), 0);
		send(client->socketfd, "\n", 1, 0);
	}
}
int main(int argc, char** argv){

	pthread_t tid;
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

	//accept connections

	for(;;){
		
		struct sock_data threadarg;		

		if((clientSock = accept(serverSock, (struct sockaddr*)&client, &length)) == -1){
			perror("Could not accept client\n");
			return -1;
		}
		threadarg.socketfd = clientSock;
		threadarg.addrinfo = &client;
		pthread_create(&tid, 0, handle_connection, &threadarg);
		
	}
	
	
	return 0;
}
