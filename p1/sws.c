/* 	sws.c - Simple Web Server
 *	CSc 361 January 29th, 2010
 *	David Audet - V00150102
 */

#include<arpa/inet.h>
#include<stdio.h>
#include<ctype.h>
#include<stdlib.h>
#include<ctype.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<time.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<pthread.h>
#include<sys/wait.h>

struct sock_data{
	int socketfd;
	struct sockaddr_in* addrinfo;
};


void print_log(int seqNo, char* clientAddr, int port, char* message, char* response, char* resource);
void handle_connection(struct sock_data*, int seqNo, char* path);

void print_log(int seqNo, char* clientAddr, int port, char* message, char* response, char* resource){
	time_t time_val;
	struct tm* tmstring;
	char timestring[256];
	time(&time_val);
	tmstring = localtime(&time_val);
	strftime(timestring, sizeof(timestring), "%Y %b %d %T", tmstring);
	printf("Seq no. %d %s %s:%d %s; %s;\n%s\n", seqNo, timestring, clientAddr, port, message, response, resource);
	
}
void handle_connection(struct sock_data* client, int seqNo, char* path){
	
	int receivedMsgSize;
	char* request = malloc(sizeof(char)*512);
	struct stat stbuf;
	char* request_line = malloc(sizeof(char)*32);
	char* next_request = malloc(sizeof(char)*512);
	char* response = malloc(sizeof(char)*512);
	char* resource = malloc(sizeof(char)*512);
	char* method = malloc(sizeof(char)*32);
	char* protocol = malloc(sizeof(char)*32);
	
	//Header Responses
	char* bad_request = "HTTP/1.0 400 Bad Request\r";
	char* ok_request = "HTTP/1.0 200 OK\r";
	char* forbidden_request = "HTTP/1.0 403 Forbidden\r";
	char* not_found_request = "HTTP/1.0 404 Not Found\r";

	if((receivedMsgSize = recv(client->socketfd, request, 512, 0)) < 0)
		perror("Error receiving message\n");
	do{	
		if((receivedMsgSize = recv(client->socketfd, next_request, 512, 0)) < 0)
		perror("Error receiving message\n");
	}
	while(strcmp(next_request, "\n") != 0);

	if(request != NULL){
		if(strcpy(request_line, request) == NULL)
			perror("strcpy");
		method = strtok(request, " ");
	}
	//get rid of the \n character

	request_line[strlen(request_line)-1] = '\0';
	resource = strtok(NULL, " ");
	char* full_path = malloc(sizeof(char)*512);
	
	if(strcat(full_path, path) == NULL)
		perror("strcat");
	if(strcat(full_path, resource) == NULL)
		perror("strcat");
	printf("%s\n", full_path);
	protocol = strtok(NULL, " ");
	protocol[strlen(protocol)-1] = '\0';

	if((method == NULL)||(resource == NULL) || (protocol == NULL))
		response = bad_request;
	else if(strcmp(method, "\n") == 0)
		response = bad_request;
	else{	
		//make method and http version case insensitive
		int i = 0;
		while(method[i] != '\0'){
			method[i] = toupper(method[i]);
			i++;
		}
		int j = 0;
		while(j < 4){
			protocol[j] = toupper(protocol[j]);
			j++;
		}
	}

	if((strcmp(method, "GET") != 0)||(strcmp(protocol, "HTTP/1.0") != 0))
		response = bad_request;
	else
		response = ok_request;

	//check to see if the file is in the server root path
	if(resource[0] == '.')
		response = forbidden_request;
	else if(stat(full_path, &stbuf) == -1)
		response = not_found_request;
	
	print_log(seqNo, inet_ntoa(client->addrinfo->sin_addr), ntohs(client->addrinfo->sin_port), request_line, response, full_path);
	send(client->socketfd, response, strlen(response), 0);
	send(client->socketfd, "\r\n", 1, 0);
	close(client->socketfd);
}

int main(int argc, char** argv){

	int serverSock, clientSock;
	int optval = 1;
	struct sockaddr_in client, server;
	int seq = 0;
	struct stat stbuf;
	socklen_t length = sizeof(struct sockaddr);
	fd_set readfds;
	struct timeval tv;

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	FD_ZERO(&readfds);
	FD_SET(STDIN_FILENO, &readfds);
	
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
	//fill in the local ip address of the server
	server.sin_addr.s_addr = htonl(INADDR_ANY);
//	server.sin_addr.s_addr = inet_addr("10.10.1.100");
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
	struct sock_data threadarg;		
for(;;){
	if((clientSock = accept(serverSock, (struct sockaddr*)&client, &length)) == -1){
		perror("Could not accept client\n");
		return -1;
	}
	threadarg.socketfd = clientSock;
	threadarg.addrinfo = &client;
	handle_connection(&threadarg, ++seq, argv[2]);
}
/*	
	for(;;){
		if((select(STDIN_FILENO, &readfds, NULL, NULL, &tv)) == -1)
			perror("select");
		if(FD_ISSET(STDIN_FILENO, &readfds)){
			if(getchar() == 'q')
				break;
		}
	}		
*/
	close(serverSock);
	return 0;
}
