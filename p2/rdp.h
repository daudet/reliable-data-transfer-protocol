/*Reliable Data Protocol - rdp.h
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

//packet information to be sent with any messages
typedef struct packet{
	char  _magic_[7];	//CSC361
	char  _type_[4];	//DAT, ACK, SYN, FIN, RST
	unsigned short _seqno_;	//2 byte sequence number
	unsigned short _ackno_;	//2 byte acknowledgement number
	unsigned int _length_;	//4 byte length of data payload
	unsigned int _window_;	//4 byte window size for flow control
	unsigned int _chksum_; // 4 byte checksum of the packet
	char _blankline_[2];	//blank line to denote the end of header
	char _data_[900];
}packet;

int rdp_connect(int, char*, char*);
int rdp_accept(int, struct sockaddr_in*);
int rdp_send(int, char*, size_t, struct sockaddr_in*);
int rdp_recv(int, char*, size_t, struct sockaddr_in*);

int rdp_connect(int sockfd, char* addr, char* port){
	/*
		make SYN packet with the pkt type(SYN, ACK, DAT, etc.), seqno, checksum, etc.
		send over udp this packet to the server
		sendto(socket, pkt, length, flag, server ip, server port)
		start_timer
		while((recvfrom(server) != ACK) || timeout){
			send the packet again
			sendto(socket, pkt, length, flag, server ip, server port)
			restart_timer
		}
	*/	
	struct sockaddr_in recvaddr;
	struct timeval timeout;
	struct sockaddr_in client;
	socklen_t length = sizeof(struct sockaddr);

	recvaddr.sin_family = AF_INET;
	recvaddr.sin_port = htons(atoi(port));
	recvaddr.sin_addr.s_addr = inet_addr(addr);
	printf("%s, %s\n", port, addr);
	printf("Making an SYN packet ...\n");
	packet pkt, r_pkt;
	strncpy(pkt._magic_, "CSC361\0", 7);
	strncpy(pkt._type_, "SYN\0", 4);
	pkt._seqno_ = 0;
	pkt._ackno_ = 1;
	pkt._length_ = 0;
	pkt._window_ = 4;
	strncpy(pkt._blankline_ , "\n\0", 2);
	printf("Size of packet: %d\n", (int)sizeof(packet));	
	printf("Sending SYN packet to receiver ...\n");
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	//send the SYN packet
	sendto(sockfd, (char*)&pkt, sizeof(packet), 0, (struct sockaddr *)&recvaddr, sizeof(recvaddr));
	fd_set fds;
	for(;;){
		FD_ZERO(&fds);
		FD_SET(sockfd, &fds);
		if((select(sockfd+1, &fds, NULL, NULL, &timeout)) < 0)
			printf("error with select rdp\n");
		else{
			if(FD_ISSET(sockfd, &fds)){
			
				recvfrom(sockfd, &r_pkt, sizeof(packet), 0, (struct sockaddr*)&client, &length);
				if(strncmp(r_pkt._type_, "ACK", 3) == 0){
					printf("Received ACK from server\n");
					break;
				}
			}
			else{
				printf("Timeout on socket, resending SYN packet ...\n");
				sendto(sockfd, (char*)&pkt, sizeof(packet), 0, (struct sockaddr *)&recvaddr, sizeof(recvaddr));
			}
		}
	}
	return 0;	
}
int rdp_accept(int sockfd, struct sockaddr_in *client){
	/*
	while((datarecv = recvfrom(client, ....)) != -1){
		extract rdp header from client
		make response packet with pkt type, seqno, checksum etc.
		send the packet over udp to the client
		sendto(socket, pkt, length, flag, server ip, server port)
	}
	return HTTP request to RWS
	*/
	printf("RDP_ACCEPT\n");
	packet pkt, s_pkt;
	socklen_t length = sizeof(struct sockaddr);
	recvfrom(sockfd, &s_pkt, sizeof(packet), 0, (struct sockaddr*)client, &length);
		
	printf("%d\n", client->sin_port);
	//printf("Server has received %d bytes\n", recv_bytes);
	//printf("Magic: %s\nType: %s\n", s_pkt._magic_, s_pkt._type_);
	//printf("Seqno: %d\nAckno: %d\n", s_pkt._seqno_, s_pkt._ackno_);
	//printf("Length: %d\nWindow: %d\n", s_pkt._length_, s_pkt._window_);
	//printf("%s", s_pkt._blankline_);
	
	//send ACK response
	if(strncmp(s_pkt._type_, "SYN", 3) == 0){
		printf("Received SYN packet from client ...\n");
		printf("Creating ACK packet to send to client ...\n");
		strncpy(pkt._magic_, "CSC361\0", 7);
		strncpy(pkt._type_, "ACK\0", 4);
		pkt._seqno_ = 1;
		pkt._ackno_ = 1;
		pkt._length_ = 0;
		pkt._window_ = 4;
		strncpy(pkt._blankline_ , "\n\0", 2);
		printf("Sending ACK packet to client ...\n");
		sendto(sockfd, (char*)&pkt, sizeof(packet), 0, (struct sockaddr *)client, sizeof(*client));
		return 0;
	}
	else{
		printf("Did not receive the SYN packet from client\n");
		return -1;
	}
}
int rdp_send(int sockfd, char* buffer, size_t size, struct sockaddr_in* client){
	/*
		make packet with the pkt type(SYN, ACK, DAT, etc.), seqno, checksum, etc.
		send over udp this packet to the server
		sendto(sockfd, pkt, length, flag, server ip, server port)
		start_timer
		while((recvfrom(server) != ACK) || timeout){
			send the packet again
			sendto(socket, pkt, length, flag, server ip, server port)
			restart_timer
		}
	*/	
	struct timeval timeout;
	socklen_t length = sizeof(struct sockaddr);

	printf("Making a DAT packet ...\n");
	packet pkt;
	strncpy(pkt._magic_, "CSC361\0", 7);
	strncpy(pkt._type_, "DAT\0", 4);
	pkt._seqno_ = 0;
	pkt._ackno_ = 1;
	pkt._length_ = size;
	pkt._window_ = 4;
	strncpy(pkt._blankline_ , "\n\0", 2);
	strncpy(pkt._data_, buffer, size);
	printf("Size of packet: %d\n", (int)sizeof(packet));	
	printf("Sending DAT packet to receiver ...\n");
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	//send the SYN packet
	sendto(sockfd, (char*)&pkt, sizeof(packet), 0, (struct sockaddr *)client, length);
	return 0;
}
int rdp_recv(int sockfd, char* buffer, size_t size, struct sockaddr_in* client){
	/*
		if checksum not the same, error in packet
			do nothing(sender will resend after timeout)
		CHECK packet structure
		if pkt->_type_ == SYN
			rdp_create_packet(ACK
	*/
	printf("rdp_recv\n");
	packet pkt;
	socklen_t length = sizeof(struct sockaddr);
	recvfrom(sockfd, &pkt, sizeof(packet), 0, (struct sockaddr*)client, &length);
	printf("Received the following data from the server:\n");
	strncpy(buffer, pkt._data_, strlen(pkt._data_));;
	printf("%s\n", buffer);

	return 0;
}
