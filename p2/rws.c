/* 	rws.c - Reliable Web Server
 *	CSc 361 February 26, 2010
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
#include"rdp.h"

//struct to hold important connected client information
struct sock_data{
	int socketfd;
	struct sockaddr_in* addrinfo;
	char* path;
};
//function prototypes
int print_log(char* clientAddr, int port, char* method, char* resource, char* protocol, char* response, char* full_resource);
int handle_request(int sockfd, struct sock_data*, char* path);
int send_response(int socketfd, char* response, char* resource, struct sockaddr_in*);
int send_file(int socketfd, char* resource);
int handle_connection(int socketfd, char* path);

/***************print_log*********************************
 *	This function will print out the log information
 *	recording requests handled by the server.
 *	INPUT:	client request information
 *	OUTPUT: A detailed list of what the request was, who
 *			sent it, when the request was handled and
 * 			what the response was.
 ********************************************************/
int print_log(char* clientAddr, int port, char* method, char* resource, char* protocol, char* response, char* full_resource){
	time_t time_val;
	//struct to hold the time information
	struct tm* tmstring;
	char timestring[256];
	time(&time_val);
	tmstring = localtime(&time_val);
	//format the time 
	strftime(timestring, sizeof(timestring), "%Y %b %d %T", tmstring);
	printf("\n%s %s:%d %s %s %s; %s;\n%s\n", timestring, clientAddr, port, method, resource, protocol, response, full_resource);
	return 0;
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
int handle_request(int sockfd, struct sock_data* client, char* path){
	
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

	rdp_recv(sockfd, request, 128, client->addrinfo);

	printf("rws.c-->\n%s", request);

//		perror("Error receiving message\n");
//		return -1;
//	}
	//copy the original request string
	if(strncpy(request_line, request, strlen(request)) == NULL){
		perror("strncpy");
		return -1;
	}
	//check to see if there is a blank line after the request line
	if(strstr(request, "\r\n\r\n") != NULL){
	}
	//if not we need to receive more info until we read a blank line
/*	else{	
		do{
			if(rdp_recv(client->socketfd, next_request, 128, 0) < 0){
				perror("recv");
				return -1;
			}
		//if using nc need to look for newline and not \r\n	
		}while(strncmp(next_request, "\n", 1) != 0);
	}
*/	if(strlen(request) > 14){
		//parse the request for the method, resource and protocol
		if(sscanf(request_line, "%s %s %s", method, resource, protocol) != 3){
			printf("Error parsing the request header\n");
			return -1;
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
	if((strncmp(method, "GET", 3) != 0)||(strncmp(protocol, "HTTP/1.0", 8) != 0)||(strncmp(resource, "/", 1) != 0))
		response = bad_request;
	else
		response = ok_request;
	
	//now check to see if the resource is available
	if(strncmp(response, bad_request, strlen(bad_request)) != 0){
		if(strncpy(full_path, path, strlen(path)) == NULL){
			printf("could not populate the full path\n");
			return -1;
		}
		//get default index.html file
		if((strlen(resource) == 1) && (strncmp(resource, "/", 1) == 0)){
			if(strncat(full_path, "/index.html", 11) == NULL){
				perror("strncat");
				return -1;
			}
		}	
		else{
			if(strncat(full_path, resource, strlen(resource)) == NULL){
				perror("strncat");
				return -1;
			}
		}
		//check to see if the file is in the server root path
		if((strstr(resource, "../") != NULL) || (strstr(resource, "/..") != NULL))
			response = bad_request;
		else if(stat(full_path, &stbuf) == -1){
			response = not_found_request;
		}	
		//if the resource is a directory return 400
		if((stbuf.st_mode & S_IFMT) == S_IFDIR){
				response = bad_request;
		}
		//if not a bad request
		if((strncmp(response, bad_request, strlen(bad_request)) != 0) && (strncmp(response, not_found_request, strlen(not_found_request)) != 0)){
			if((open(full_path, O_RDONLY)) == -1){
				perror("open");
				response = forbidden_request;
			}
		}
	}
/*	//print the server log for the request
	if((print_log(inet_ntoa(client->addrinfo->sin_addr), ntohs(client->addrinfo->sin_port), method, resource, protocol, response, full_path)) == -1){
		printf("There was an error printing the log\n");
		return -1;
	}
*/	//send the HTTP response
	if((send_response(sockfd, response, full_path, client->addrinfo)) == -1){
		printf("There was an error sending the response to the client\n"); 
		return -1;
	}
	//free all dynamically allocated memory
	free(request);
	free(request_line);
	free(next_request);
	free(resource);
	free(method);
	free(full_path);
	free(protocol);
	return 0;
}
/***********send_response***********************************
 * 	This function will send the HTTP response to the client
 *	INPUT:	client socket file descriptor, response message
 *			and the requested resource to serve
 *	OUTPUT:	The HTTP response message including HTTP ver.
 *			and status code, date, content-type, followed
 *			by a blank line and the request resource
 **********************************************************/
int send_response(int socketfd, char* response, char* resource, struct sockaddr_in* client){
	time_t t;
	//char array to hold the date string
	char* date = malloc(sizeof(char)*48);
	//char array to hold the file extension of the resource
	char* extension = malloc(sizeof(char)*32);
	char* content_type;
	char* send_buffer = calloc(32, sizeof(char));
	//zero out the date string
	if((memset(date, '\0', 48)) == NULL){
		printf("error zeroing out path buffer\n");
		return -1;
	}
	//send the HTTP version and status
	if(strncat(send_buffer, response, strlen(response)) == NULL){
		perror("strncat");
		return -1;
	}
	if(strncat(send_buffer, "\r\n", 2) == NULL){
		perror("strncat");
		return -1;
	}
	if((time(&t)) == (time_t)-1){
		printf("There was an error getting the current time\n");
		return -1;
	}
	//convert the time to GMT
	char* gmt_time = asctime(gmtime(&t));
	if(strncpy(date, "Date: ", 6) == NULL){
		perror("strncpy");
		return -1;
	}
	if(strncat(date, gmt_time, strlen(gmt_time)) == NULL){
		perror("strncpy");
		return -1;
	}
	//remove '\n' char from end of date string
	date[strlen(date)-1] = ' ';
	if(strncat(date, "GMT\r\n", 6) == NULL){
		perror("strncpy");
		return -1;
	}
	//send the date header
	if(strncat(send_buffer, date, strlen(date)) == NULL){
		perror("strncpy");
		return -1;
	}
	if(sscanf(resource, "%*[^/]%*[^.]%*c%s", extension) != 1){
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
	if(strncat(send_buffer, content_type, strlen(content_type)) == NULL){
		perror("strnlen");
		return -1;
	}
	if(strncat(send_buffer, "\r\n", 2) == NULL){
		perror("strnlen");
		return -1;
	}
	rdp_send(socketfd, send_buffer, strlen(send_buffer), client);
/*	//send the file only if the response is 200 OK
	if(strstr(response, "200") != NULL)
		if((send_file(socketfd, resource)) == -1){
			printf("There was an error sending the file to the client\n");
			return -1;
		}
*/	//free dynamically allocated memory
	free(date);
	free(extension);
	free(send_buffer);
	return 0;
}
/***************send_file****************************
 *	This function will send the requested resource
 *	to the client
 *	INPUT:	client socket file descriptor and full
 *			resource path
 *	OUTPUT:	the resource as sent in 1460 byte chunks
 *
 ****************************************************/
int send_file(int socketfd, char* resource){
	//buffer to hold the info read from the resource file
	char* buffer = malloc(sizeof(char)*900);
	//struct to hold information about the resource
	struct stat stbuf;
	int amount_read = 0;
	int amount_sent = 0;
	int total_sent = 0;
	int buffer_fd;
	//check to see if the file exists
	if(stat(resource, &stbuf) == -1){
		printf("file does not exist!\n");
		return -1;
	}
	//open the file for reading
	if((buffer_fd = open(resource, O_RDONLY)) == -1){
		perror("open");
		return -1;
	}
	//read 1460 bytes from the file
/*	while((amount_read = read(buffer_fd, buffer, 900)) > 0){
		if((amount_sent = rdp_send(socketfd, buffer, amount_read)) < 0){
			perror("send");
			return -1;
		}
		else
			total_sent += amount_sent;
	}
*/	//check to see if the whole file was sent successfully
	if(total_sent == (int)stbuf.st_size)
		printf("The file was sent successfully\n");
	else{
		printf("The file was not sent successfully\n");
		close(buffer_fd);
		free(buffer);
		return -1;
	}
	//close the resource file and free the memory of the buffer
	close(buffer_fd);
	free(buffer);
	return 0;
}
/**********************handle_connection*************************
 *	This function will accept the client connection on the 
 *	socket
 *	INPUT:	client socket file descriptor, server's path and 
 * 			request sequence number
 *	OUTPUT:	call to helper function handle_request to deal
 *			with the getting the HTTP request and returning 
 *			the HTTP response
 ***************************************************************/
int handle_connection(int socketfd, char* path){
	//structure to hold the client socket info	
	struct sock_data client_info;
	struct sockaddr_in client;
	//accept the connection from the client
	for(;;){
		if((rdp_accept(socketfd, &client)) == 0){
			printf("Connection successful!\n");
			break;
		}
	}
	//populate the sock_data structure
	client_info.addrinfo = &client;
	if((handle_request(socketfd, &client_info, path)) == -1){
		printf("There was an error handling the client request\n");
		return -1;
		if((close(socketfd)) == -1){
			printf("Could not close the client socket\n");
		}
	}
	return 0;
}
int main(int argc, char** argv){

	int serverSock;
	int optval = 1;
	struct sockaddr_in server;
	struct stat stbuf;
	fd_set fds;
	
	//check for proper execution of sws
	if(argc != 3){
		printf("Incorrect number of parameters!\nUsage: ./sws <port> <directory>\n");
		return -1;
	}
	//create a stream socket
	if((serverSock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
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
	//server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons(atoi(argv[1]));

	//Bind the socket
	if((bind(serverSock, (struct sockaddr*)&server, sizeof(server))) == -1){
		perror("Could not bind socket");
		return -1;
	}
	//Check for valid port number(max tcp port is 65535
	if((atoi(argv[1]) < 1025) || (atoi(argv[1]) > 65535)){
		printf("Invalid port number for this server\n");
		return -1;
	}
	//Check for a valid serving directory
	if((stat(argv[2], &stbuf)) == -1){
		printf("Invalid directory to serve\n");
		return -1;
	}
	printf("rws is running on UDP port %d and serving %s\n", atoi(argv[1]), argv[2]);
	printf("press 'q' to quit ...\n");
	
	for(;;){
		//reset the list of fds
		FD_ZERO(&fds);
		//add stdin to the list of fds to watch
		FD_SET(STDIN_FILENO, &fds);
		//add serverSock to the list of fds to watch
		FD_SET(serverSock, &fds);

		if((select(serverSock+1, &fds, NULL, NULL, NULL)) < 0)
			perror("select->rws.c");
		else{	
			//if there was input from stdin check to see if it was 'q'
			if(FD_ISSET(STDIN_FILENO, &fds)){
				if(((fgetc(stdin) == 'q') && (fgetc(stdin) == '\n'))){
					printf("The server is going down now...\n");
					break;
				}
			}
			//if there was activity on the socket, handle the connection
			if(FD_ISSET(serverSock, &fds)){
				if((handle_connection(serverSock, argv[2])) == -1){
					printf("There was an error handling the incoming connection\n");
					return -1;
				}
			}
		}	
	}
	if((close(serverSock)) == -1){
		printf("There was an error closing the server socket\n");
		return -1;
	}
	return 0;
}
