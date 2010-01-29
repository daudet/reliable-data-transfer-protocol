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
//struct to hold important connected client information
struct sock_data{
	int socketfd;
	struct sockaddr_in* addrinfo;
	char* path;
};
//function prototypes
void print_log(int seqNo, char* clientAddr, int port, char* method, char* resource, char* protocol, char* response, char* full_resource);
void handle_request(struct sock_data*, int seqNo, char* path);
void handle_connection(int socketfd, char* path, int seqNo);
void send_response(int socketfd, char* response, char* resource);
void send_file(int socketfd, char* resource);

/***************print_log*********************************
 *	This function will print out the log information
 *	recording requests handled by the server.
 *	INPUT:	client request information
 *	OUTPUT: A detailed list of what the request was, who
 *			sent it, when the request was handled and
 * 			what the response was.
 ********************************************************/
void print_log(int seqNo, char* clientAddr, int port, char* method, char* resource, char* protocol, char* response, char* full_resource){
	time_t time_val;
	//struct to hold the time information
	struct tm* tmstring;
	char timestring[256];
	time(&time_val);
	tmstring = localtime(&time_val);
	//format the time 
	strftime(timestring, sizeof(timestring), "%Y %b %d %T", tmstring);
	printf("Seq no. %d %s %s:%d %s %s %s; %s;\n%s\n", seqNo, timestring, clientAddr, port, method, resource, protocol, response, full_resource);
}
/************handle_request*****************************
 *	This function will parse the HTTP request and 
 *	create the HTTP response.
 *	INPUT:	client socket information, sequence
 *			number of the request and the server's
 *			served directory path.
 *	OUTPUT:	once the information from the request
 *			is gathered, the request will be sent
 * 			to the client through helper functions
 *			and the connection will be closed
 *******************************************************/
void handle_request(struct sock_data* client, int seqNo, char* path){
	
	struct stat stbuf;
	char* request = malloc(sizeof(char)*128);
	char* request_line = malloc(sizeof(char)*128);
	char* next_request = malloc(sizeof(char)*128);
	char* resource = malloc(sizeof(char)*128);
	char* method = malloc(sizeof(char)*128);
	char* protocol = malloc(sizeof(char)*128);
	char* full_path = malloc(sizeof(char)*128);
	
	//zero out the full_path
	if((memset(full_path, '\0', 128)) == NULL){
		printf("error zeroing out path buffer\n");
	}
	//Header Responses
	char* response;
	char* bad_request = "HTTP/1.0 400 Bad Request";
	char* ok_request = "HTTP/1.0 200 OK";
	char* forbidden_request = "HTTP/1.0 403 Forbidden";
	char* not_found_request = "HTTP/1.0 404 Not Found";

	if(recv(client->socketfd, request, 128, 0) < 0){
		perror("Error receiving message\n");
		return;
	}
	//copy the original request string
	if(strncpy(request_line, request, strlen(request)) == NULL){
		perror("strncpy");
		return;
	}
	//check to see if there is a blank line after the request line
	if(strstr(request, "\r\n\r\n") != NULL){
	}
	//if not we need to receive more info until we read a blank line
	else{	
		do{
			if(recv(client->socketfd, next_request, 128, 0) < 0){
				perror("recv");
				return;
			}
		//if using nc need to look for newline and not \r\n	
		}while(strncmp(next_request, "\n", 1) != 0);
	}
	if(strlen(request) > 14){
		//parse the request for the method, resource and protocol
		if(sscanf(request_line, "%s %s %s", method, resource, protocol) != 3){
			printf("Error parsing the request header\n");
			return;
		}
		if((method == NULL)||(resource == NULL) || (protocol == NULL))
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
	}
	else
		response = bad_request;

	//Check for correct method and protocol	
	if((strncmp(method, "GET", 3) != 0)||(strncmp(protocol, "HTTP/1.0", 8) != 0))
		response = bad_request;
	else
		response = ok_request;
	
	//now check to see if the resource is available
	if(strncmp(response, bad_request, strlen(bad_request)) != 0){
		if(strncpy(full_path, path, strlen(path)) == NULL){
			printf("could not populate the full path\n");
			return;
		}
		//get default index.html file
		if((strlen(resource) == 1) && (strncmp(resource, "/", 1) == 0)){
			if(strncat(full_path, "/index.html", 11) == NULL){
				perror("strncat");
				return;
			}
		}	
		else{
			if(strncat(full_path, resource, strlen(resource)) == NULL){
				perror("strncat");
				return;
			}
		}
		//check to see if the file is in the server root path
		if((strstr(resource, "../") != NULL) || (strstr(resource, "/..") != NULL))
			response = bad_request;
		else if(stat(full_path, &stbuf) == -1)
			response = not_found_request;

		if((open(full_path, O_RDONLY)) == -1){
			perror("open");
			response = forbidden_request;
		}
	}
	//print the server log for the request
	print_log(seqNo, inet_ntoa(client->addrinfo->sin_addr), ntohs(client->addrinfo->sin_port), method, resource, protocol, response, full_path);
	//send the HTTP response
	send_response(client->socketfd, response, full_path);
	//free all dynamically allocated memory
	free(request);
	free(request_line);
	free(next_request);
	free(resource);
	free(method);
	free(full_path);
	free(protocol);
	close(client->socketfd);
}
/***********send_response***********************************
 * 	This function will send the HTTP response to the client
 *	INPUT:	client socket file descriptor, response message
 *			and the requested resource to serve
 *	OUTPUT:	The HTTP response message including HTTP ver.
 *			and status code, date, content-type, followed
 *			by a blank line and the request resource
 **********************************************************/
void send_response(int socketfd, char* response, char* resource){
	time_t t;
	//char array to hold the date string
	char* date = malloc(sizeof(char)*48);
	//char array to hold the file extension of the resource
	char* extension = malloc(sizeof(char)*32);
	char* content_type;
	//zero out the date string
	if((memset(date, '\0', 48)) == NULL){
		printf("error zeroing out path buffer\n");
	}
	//send the HTTP version and status
	if(send(socketfd, response, strlen(response), 0) < 0){
		perror("send");
		return;
	}	
	//send \r\n to end the header line
	if(send(socketfd, "\r\n", 2, 0) < 0){
		perror("send");
		return;
	}
	//fill the time_t parameter with the current time
	time(&t);
	//convert the time to GMT
	char* gmt_time = asctime(gmtime(&t));
	if(strncpy(date, "Date: ", 6) == NULL){
		perror("strncpy");
		return;
	}
	if(strncat(date, gmt_time, strlen(gmt_time)) == NULL){
		perror("strncpy");
		return;
	}
	//remove '\n' char from end of date string
	date[strlen(date)-1] = ' ';
	if(strncat(date, "GMT\r\n", 6) == NULL){
		perror("strncpy");
	}
	//send the date header
	if(send(socketfd, date, strlen(date), 0) < 0){
		perror("send");
		return;
	}
	if(sscanf(resource, "%*[^'.'].%s", extension) != 1){
		printf("There was an error scanning the extension\n");
	}
	//match the content type to the file extension
	if(strncmp(extension, "html", 4) == 0)
		content_type = "Content-Type: text/html\r\n";
	else if(strncmp(extension, "jpg", 4) == 0)
		content_type = "Content-Type: image/jpeg\r\n";
	else if(strncmp(extension, "txt", 3) == 0)
		content_type = "Content-Type: text/plain\r\n";
	else if(strncmp(extension, "pdf", 3) == 0)
		content_type = "Content-Type: text/pdf\r\n";
	else
		content_type = "Content-Type: unknown\r\n";
	//send the content type
	if(send(socketfd, content_type, strlen(content_type), 0) < 0){
		perror("send");
		return;
	}
	//send \r\n to end the content type header
	if(send(socketfd, "\r\n", 2, 0) < 0){
		perror("send");
		return;
	}
	//send the file
	send_file(socketfd, resource);
	//free dynamically allocated memory
	free(date);
	free(extension);
}
/***************send_file****************************
 *	This function will send the requested resource
 *	to the client
 *	INPUT:	client socket file descriptor and full
 *			resource path
 *	OUTPUT:	the resource as sent in 1460 byte chunks
 *
 ****************************************************/
void send_file(int socketfd, char* resource){
	//buffer to hold the info read from the resource file
	char* buffer = malloc(sizeof(char)*1460);
	//struct to hold information about the resource
	struct stat stbuf;
	int amount_read = 0;
	int amount_sent = 0;
	int total_sent = 0;
	int buffer_fd;
	//check to see if the file exists
	if(stat(resource, &stbuf) == -1){
		printf("file does not exist!\n");
		return;
	}
	//open the file for reading
	if((buffer_fd = open(resource, O_RDONLY)) == -1){
		perror("open");
		return;
	}
	//read 1460 bytes from the file
	while((amount_read = read(buffer_fd, buffer, 1460)) > 0){
		if((amount_sent = send(socketfd, buffer, amount_read, 0)) < 0){
			perror("send");
			return;
		}
		else
			total_sent += amount_sent;
	}
	//check to see if the whole file was sent successfully
	if(total_sent == (int)stbuf.st_size)
		printf("The file was sent successfully\n");
	else
		printf("The file was not sent successfully\n");
	//close the resource file and free the memory of the buffer
	close(buffer_fd);
	free(buffer);
}
/**********************handle_connection*************************
 *	This function will accept the client connection on the 
 *	socket
 *	INPUT:	client socket file descriptor, server's path and 
 * 			request sequence number
 *	OUTPUT:	will call helper function handle_request to deal
 *			with the getting the HTTP request and returning 
 *			the HTTP response
 ***************************************************************/
void handle_connection(int socketfd, char* path, int seqNo){
	//structure to hold the client socket info	
	struct sock_data threadarg;		
	struct sockaddr_in client;
	int clientSock;
	socklen_t length = sizeof(struct sockaddr);
	//accept the connection from the client
	if((clientSock = accept(socketfd, (struct sockaddr*)&client, &length)) == -1){
		perror("Could not accept client\n");
		return;
	}
	//populate the sock_data structure
	threadarg.socketfd = clientSock;
	threadarg.addrinfo = &client;
	handle_request(&threadarg, seqNo, path);
}
int main(int argc, char** argv){

	int serverSock;
	int optval = 1;
	int seqNo = 0;
	struct sockaddr_in server;
	struct stat stbuf;
	fd_set fds;
	struct timeval tv;
	
	//check for proper execution of sws
	if(argc != 3){
		printf("Incorrect number of parameters!\nUsage: ./sws <port> <directory>\n");
		return -1;
	}
	//create a stream socket
	if((serverSock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("Could not create the socket");
		return -1;
	}
	//set the socket to reuse ports if server is run on same port successively
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

	//Bind the socket
	if((bind(serverSock, (struct sockaddr*)&server, sizeof(server))) == -1){
		perror("Could not bind socket");
		return -1;
	}
	//Listen on the socket with backlog of 5 for the listen queue
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

	for(;;){
		//timeout value for select(): 1 second 0 milliseconds
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		//initialize the fd_set variable
		FD_ZERO(&fds);
		//add stdin to the list of fds to watch
		FD_SET(STDIN_FILENO, &fds);
		//add serverSock to the list of fds to watch
		FD_SET(serverSock, &fds);

		if((select(serverSock+1, &fds, NULL, NULL, &tv)) < 0)
			perror("select");
		else{	
			//if there was input from stdin check to see if it was 'q'
			if(FD_ISSET(STDIN_FILENO, &fds)){
				if(getchar() == 'q'){
					printf("The server is going down now...\n");
					break;
				}
			}
			//if there was activity on the socket, handle the connection
			if(FD_ISSET(serverSock, &fds)){
				handle_connection(serverSock, argv[2], ++seqNo);
			}
		}	
	}
	close(serverSock);
	return 0;
}
