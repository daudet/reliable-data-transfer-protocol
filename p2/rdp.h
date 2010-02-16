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
	char _data_[991];
}packet;

int rdp_connect(int, char*, char*);
struct sockaddr_in rdp_accept(int socketfd);
int rdp_send();
int rdp_receive();
packet rdp_create_packet(char*,char*, unsigned short, unsigned short);

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
	recvaddr.sin_family = AF_INET;
	recvaddr.sin_port = htons(atoi(port));
	recvaddr.sin_addr.s_addr = inet_addr(addr);
	printf("%s, %s\n", port, addr);
	packet pkt;
	strncpy(pkt._magic_, "CSC361\0", 7);
	strncpy(pkt._type_, "ACK\0", 4);
	pkt._seqno_ = 123;
	pkt._ackno_ = 45;
	pkt._length_ = 32558;
	pkt._window_ = 2048;
	strncpy(pkt._blankline_ , "\n\0", 2);
	strncpy(pkt._data_, "<html>\nThis is a sent HTML page\n</html>\n\0",41);
	printf("Size of packet: %d\n", (int)sizeof(packet));	
	sendto(sockfd, (char*)&pkt, sizeof(packet), 0, (struct sockaddr *)&recvaddr, sizeof(recvaddr));
	
	return 0;	
}
struct sockaddr_in rdp_accept(int socketfd){
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
	packet pkt;
	struct sockaddr_in client;
	socklen_t length = sizeof(struct sockaddr);
	int recv_bytes = recvfrom(socketfd, &pkt, sizeof(packet), 0, (struct sockaddr*)&client, &length);
	
	printf("Server has received %d bytes\n", recv_bytes);
	printf("Magic: %s\nType: %s\n", pkt._magic_, pkt._type_);
	printf("Seqno: %d\nAckno: %d\n", pkt._seqno_, pkt._ackno_);
	printf("Length: %d\nWindow: %d\n", pkt._length_, pkt._window_);
	printf("%s", pkt._blankline_);
	printf("%s", pkt._data_);
	
	//send ACK response

	return client;
}
int rdp_send(){
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
	*/	
return 0;
}
int rdp_receive(){
	/*
		if checksum not the same, error in packet
			do nothing(sender will resend after timeout)
		CHECK packet structure
		if pkt->_type_ == SYN
			rdp_create_packet(ACK
	*/
	return 0;
}
packet rdp_create_packet(char* protocol, char* type, unsigned short seqNo, unsigned short ackNo){
	packet pkt;
	
	strncpy(pkt._magic_, "CSC361\0", 7);
	strncpy(pkt._type_, "ACK\0", 4);

	return pkt;
}
