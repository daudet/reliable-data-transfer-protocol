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
struct packet{
	char* _magic_;
	char* _type_;
	short _seqno_;
	short _ackno_;
	short _length_;
	short _size_;
}

int rdp_connect();
int rdp_listen();
int rdp_send();
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
int rdp_send(int sockfd, struct mine* recvaddr){
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
}

int rdp_receive(){
	/*
		


	*/
}
