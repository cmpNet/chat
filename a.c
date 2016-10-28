#include <stdio.h>	//for printf
#include <string.h> //memset
#include <sys/socket.h>	//for socket ofcourse
#include <stdlib.h> //for exit(0);
#include <errno.h> //For errno - the error number
#include <netinet/udp.h>	//Provides declarations for udp header
#include <netinet/ip.h>	//Provides declarations for ip header
#include <netinet/tcp.h>
#include <time.h>
#include <netinet/if_ether.h>

#define PACKETLEN 4096

typedef enum {
	FIN, SYN, RST, PSH, ACK, URG, NONE
}FLAG;

typedef enum{
    S_START, S_CLOSED, S_CONNECTING, S_LISTEN, S_SYN_SENT, S_SYN_ACK_SENT,
    S_SYN_RECEIVED, S_ESTABLISHED, S_FIN_WAIT_1, S_FIN_WAIT_2, S_CLOSE_WAIT,
    S_TIME_WAIT, S_CLOSING, S_LAST_ACK
} state_t;

typedef struct tcb{
	in_addr_t our_ipaddr;
	in_addr_t their_ipaddr;
	int our_port;
	int their_port;
	int  our_seqnr;
	int their_seqnr;
	int acknr;
	int sockfd;
	int revfd;
	char buffer[PACKETLEN];
	state_t state;
} tcb_t;

int s;
char buf[PACKETLEN];
tcb_t tcpblock;

char* create_packet(in_addr_t source_ip, in_addr_t dst_ip, FLAG flag, int seqnr, int srcport, int dstport, char* data, int data_len) {
	char* packet = (char*)malloc(PACKETLEN), *packet_data;
	memset (packet, 0, 4096);
	struct iphdr *iph = (struct iphdr *) packet;
	struct tcphdr *tcph = (struct tcphdr *) (packet + sizeof (struct iphdr));
	packet_data = packet + sizeof(struct iphdr) + sizeof(struct tcphdr);

	strcpy(packet_data, data);
	packet_data[data_len] = '\0';

	iph->ihl = 5;
	iph->version = 4;
	iph->tos = 0;
	iph->tot_len = sizeof (struct iphdr) + sizeof (struct tcphdr) + data_len + 13;
	iph->id = htonl (54321);	
	iph->frag_off = 0;
	iph->ttl = 255;
	iph->protocol = IPPROTO_TCP;
	iph->check = 0;		
	iph->saddr = source_ip;	
	iph->daddr = dst_ip;

	tcph->source = htons (srcport);
	tcph->dest = htons (dstport);
	tcph->seq = htonl(seqnr);
	tcph->ack_seq = 0;
	tcph->doff = 5;
	tcph->window = htons(7);
	tcph->check = 0;
	tcph->urg = 0;
	if (flag == FIN) tcph->fin = 1;
	else if (flag == SYN) tcph->syn = 1;
	else if (flag == ACK) tcph->ack = 1;
	else if (flag == RST) tcph->rst = 1;
	else if (flag == PSH) tcph->psh = 1;
	else if (flag == URG) tcph->urg = 1;

	return packet;
}

int send_packet(char* packet) {
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(tcpblock.their_port);
	sin.sin_addr.s_addr = tcpblock.their_ipaddr;

	if (sendto(tcpblock.sockfd, packet, PACKETLEN, 0, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
		return -1;
	} else {
		return 1;
	}
}

int send_syn(int seqnr) {
	char* packet = create_packet(tcpblock.our_ipaddr, tcpblock.their_ipaddr, SYN, seqnr, tcpblock.our_port, tcpblock.their_port, "", 0);
	
	if (send_packet(packet) < 0) {
		perror("Send SYN failed.\n");
		return -1;
	} else {
		printf("Send SYN.\n");
	}

	return 0;
}

int send_ack(int seqnr) {
	char* packet = create_packet(tcpblock.our_ipaddr, tcpblock.their_ipaddr, ACK, seqnr, tcpblock.our_port, tcpblock.their_port, "", 0);

	if (send_packet(packet) < 0) {
		perror("Send ACK failed.\n");
		return -1;
	} else {
		printf("Send ACK.\n");
	}

	return 0;
}

int send_data(int seqnr, char* data, int len) {
	char* packet = create_packet(tcpblock.our_ipaddr, tcpblock.their_ipaddr, NONE, seqnr, tcpblock.our_port, tcpblock.their_port, data, len);

	if (send_packet(packet) < 0) {
		perror("Send data failed.\n");
		return -1;
	} else {
		printf("Send data.\n");
	}

	return 0;
}

int send_fin(int seqnr) {
	char* packet = create_packet(tcpblock.our_ipaddr, tcpblock.their_ipaddr, FIN, seqnr, tcpblock.our_port, tcpblock.their_port, "", 0);

	if (send_packet(packet) < 0) {
		perror("Send FIN failed.\n");
		return -1;
	} else {
		printf("Send FIN.\n");
	}

	return 0;
}

int wait_for_ack(int seq) {
	time_t begin_time;
	time_t end_time;
	int Timeout = 1000;
	time(&begin_time);
	int sockfd;

	while (1) {
		ssize_t n = recv(tcpblock.revfd, buf, sizeof(buf), 0);
		if (n == -1) {
            printf("recv error!\n");
            break;
        } else if (n == 0) {
			continue;
		}
		printf("Received.\n");
		struct iphdr* ip = (struct iphdr*)buf;
		size_t iplen =  sizeof(struct iphdr);
		struct tcphdr* tcp = (struct tcphdr*)(buf + iplen + 20);

		unsigned char* p = (unsigned char*)&ip->saddr;
		printf("Source IP\t: %u.%u.%u.%u\n",p[0],p[1],p[2],p[3]);
		p = (unsigned char*)&ip->daddr;
		printf("Destination IP\t: %u.%u.%u.%u\n",p[0],p[1],p[2],p[3]);

		if (ip->protocol == IPPROTO_TCP && ip->saddr == tcpblock.their_ipaddr && tcp->seq == seq)
			return 1;
	}
	return -1;
}

int tcp_socket(char* our_ip, char* their_ip, int our_port, int their_port) {
	tcpblock.state = S_START;
	tcpblock.our_ipaddr = inet_addr(our_ip);
	tcpblock.their_ipaddr = inet_addr(their_ip);
	tcpblock.our_port = our_port;
	tcpblock.their_port = their_port;
	tcpblock.our_seqnr = 0;
	tcpblock.their_seqnr = 0;
	tcpblock.acknr = 0;
	tcpblock.sockfd = -1;
	tcpblock.revfd = -1;
	memset(tcpblock.buffer, 0, sizeof(tcpblock.buffer));

	tcpblock.sockfd = socket (AF_INET, SOCK_RAW, IPPROTO_TCP);
	if (tcpblock.sockfd == -1) {
		perror("Failed to creare sending raw socket.\n");
		return -1;
	}

	tcpblock.revfd = socket(PF_PACKET,  SOCK_DGRAM, htons(ETH_P_IP));
	if (tcpblock.revfd == -1) {
		perror("Failed to create receiving raw socket.\n");
		return -1;
	}
}

int tcp_connect(char* source_ip, char* dst_ip, int srcport, int dstport) {
	if (tcpblock.sockfd == -1){
		perror("Our sockfd is null.\n");
		return -1;
	}

	if (tcpblock.revfd == -1) {
		perror("Received sockfd is null.\n");
		return -1;
	}

	if (tcpblock.state == S_START) {
		tcpblock.state = S_CONNECTING;

		send_syn(1);
		if (wait_for_ack(2) < 0)
			return -1;
		send_ack(3);
		return 1;
	} else {
		return -1;
	}
}

int tcp_read() {

}

int tcp_write() {

}

int tcp_close() {

}

int main (void) {
	while (1) {
		tcp_socket("127.0.0.1", "127.0.0.1", 6666, 6346);
		tcp_connect("127.0.0.1", "127.0.0.1", 6666, 6346);
	}
	return 0;
}
