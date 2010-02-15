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
	short _seqno_;	//byte sequence number
	short _ackno_;	//acknowledgement number
	short _length_;	//length of data payload
	short _size_;	//window size for flow control
	int   _blankline_;	//blank line to denote the end of header
}packet;

int rdp_connect();
int rdp_listen();
int rdp_send(int, char*, char*);
int rdp_receive();

int rdp_connect(){
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
		
		return 0;
	*/
}
int rdp_listen(){
	/*
	while((datarecv = recvfrom(client, ....)) != -1){
		extract rdp header from client
		make response packet with pkt type, seqno, checksum etc.
		send the packet over udp to the client
		sendto(socket, pkt, length, flag, server ip, server port)
	}
	return HTTP request to RWS
	*/	
}
int rdp_send(int sockfd, char* addr, char* port){
	/*
		make packet with the pkt type(SYN, ACK, DAT, etc.), seqno, checksum, etc.
		send over udp this packet to the server
		sendto(socket, pkt, length, flag, server ip, server port)
		start_timer
		while((recvfrom(server) != ACK) || timeout){
			send the packet again
			sendto(socket, pkt, length, flag, server ip, server port)
			restart_timer
		}
		
		return 0;
	*/
	struct sockaddr_in recvaddr;
	recvaddr.sin_family = AF_INET;
	recvaddr.sin_port = htons(atoi(port));
	recvaddr.sin_addr.s_addr = inet_addr(addr);
	printf("%s, %s\n", port, addr);
	char* buffer = malloc(sizeof(char)*1024);
	packet pkt;
	strncpy(pkt._magic_, "CSC361\0", 7);
	strncpy(pkt._type_, "ACK\0", 4);
	pkt._seqno_ = 123;
	pkt._ackno_ = 45;
	char* message = "hello server";
	
	sendto(sockfd, (char*)&pkt, sizeof(packet), 0, (struct sockaddr *)&recvaddr, sizeof(recvaddr));
}
int rdp_receive(){
	/*
	
	*/
}
