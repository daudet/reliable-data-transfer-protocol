#include<arpa/inet.h>
#include<stdio.h>
#include<netinet/in.h>
#include<netdb.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<time.h>
#include<fcntl.h>

int main(int argc, char**argv){

	int sock, retval;
	int true = 1;
	struct sockaddr_in client, server;
	socklen_t length = sizeof(struct sockaddr);

	if(argc != 2){
		printf("Incorrect number of parameters!\nUsage: ./server <port number>\n");
		return 1;
	}

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		printf("Could not create a socket\n");
		return 1;
	}
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&true, sizeof(int)) == -1){
		printf("Could not set socket options\n");
		return 1;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(atoi(argv[1]));


	if(((bind(sock, (struct sockaddr *)&server, sizeof(server))) == -1) || (listen(sock, 10) == -1)){
		printf("Could not bind or listen on port %d\n", atoi(argv[1]));
		return 1;
	}

	printf("sws is running on TCP port %d and serving %s\n", atoi(argv[1]), argv[2]);
	printf("press 'q' to quit ...\n");
	
	if((retval = accept(sock, (struct sockaddr *)&client, &length)) == -1){
		printf("Could not accept client\n");
		return 1;
	}
	
	printf("Received connection from client %s and port %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
			
	char* greeting = "Thanks for the message!!\n";
	char* reply = malloc(sizeof(char)*512);
	int receivedMsgSize;
	time_t t_time;
	struct tm* tmstring;
	int seq = 0;
	char timestring[256];
	send(retval, greeting, strlen(greeting), 0);
	for(;;){	
		if((receivedMsgSize = recv(retval, reply, 512, 0)) < 0){
			printf("Error receiving message\n");
			}
			time(&t_time);
			tmstring = localtime(&t_time);
			strftime(timestring, sizeof(timestring), "%Y %b %d %T", tmstring);  
			printf("Seq no. %d %s %s:%d %s\n", ++seq, timestring, inet_ntoa(client.sin_addr), ntohs(client.sin_port), reply);
			char buffer[2048];
			int fd = open("/home/david/testing.txt", O_RDONLY);
			int n;
			while((n=read(fd, buffer, 2048))>0){
				send(retval, buffer, n, 0);
			}
			close(fd);
			//send(retval, reply, strlen(greeting), 0);
	}
	close(sock);
	return 0;
}
