#include<arpa/inet.h>
#include<stdio.h>
#include<netinet/in.h>
#include<netdb.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>

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


	if(((bind(sock, (struct sockaddr *)&server, sizeof(server))) == -1) || (listen(sock, 50) == -1)){
		printf("Could not bind or listen on port %d\n", atoi(argv[1]));
		return 1;
	}

	printf("Waiting for data on port %d...\n", atoi(argv[1]));
	
	for(;;){

		if((retval = accept(sock, (struct sockaddr *)&client, &length)) == -1){
			printf("Could not accept client\n");
			return 1;
		}

		printf("Received connection from client %s and port %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
			
		char* greeting = "Thanks for the message!!\n";
		char* reply = malloc(sizeof(char)*512);
		int receivedMsgSize;
		send(retval, greeting, strlen(greeting), 0);
		
		if((receivedMsgSize = recv(retval, reply, 512, 0)) < 0)
			printf("Error receiving message\n");
		printf("message from client: %s\n", reply);
	}
	close(sock);
	return 0;
}
