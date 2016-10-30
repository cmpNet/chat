#include "common.h"

char* create_packet(in_addr_t source_ip, in_addr_t dst_ip, FLAG flag, int seqnr, int srcport, int dstport, char* data, int data_len) {
	char* packet = (char*)malloc(PACKETLEN), *packet_data;
	memset (packet, 0, 4096);
	//struct iphdr *iph = (struct iphdr *) packet;
	struct tcphdr *tcph = (struct tcphdr *) packet;
	packet_data = packet + sizeof(struct tcphdr);

	strcpy(packet_data, data);
	packet_data[data_len] = '\0';

	/*
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
	*/

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
	else if (flag == SYN_ACK) {tcph->syn = 1; tcph->ack = 1;}

	return packet;
}

int send_packet(char* packet, in_addr_t ip, int port, int sockfd) {
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = ip;

	if (sendto(sockfd, packet, PACKETLEN, 0, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
		return -1;
	} else {
		return 1;
	}
}

int my_recv(tcb_t* tcpblock) {
	int count = 0;
	while (1) {
		ssize_t n = recv(tcpblock->revfd, tcpblock->buffer, sizeof(tcpblock->buffer), 0);
		if (n == -1) {
	        // printf("recv error!\n");
	        return -1;
	    } else if (n == 0) {
			return 0;
		}
		struct iphdr *ip = (struct iphdr*)(tcpblock->buffer);
		unsigned char* p = (unsigned char*)&ip->saddr;
		if (ip->daddr != tcpblock->our_ipaddr || ip->protocol != IPPROTO_TCP)
			continue;
		struct tcphdr *tcp = (struct tcphdr *)(tcpblock->buffer + sizeof(struct iphdr));
		//printf("Source IP: %u.%u.%u.%u\n", p[0], p[1], p[2], p[3]);
		//printf("Port: %d %d\n", htons(tcp->dest), tcpblock->our_port);
		if (htons(tcp->dest) != tcpblock->our_port)
			continue;
		//printf("%d\n", htons(tcp->dest));
		break;
	}
	return 1;
}

int send_syn_ack(in_addr_t our_ipaddr, in_addr_t their_ipaddr, int our_port, int their_port, int seqnr, int sockfd) {
	char* packet = create_packet(our_ipaddr, their_ipaddr, SYN_ACK, seqnr, our_port, their_port, "", 0);

	if (send_packet(packet, their_ipaddr, their_port, sockfd) < 0) {
		perror("Send SYN_ACK failed.\n");
		return -1;
	} else {
		// printf("Send SYN_ACK.\n");
	}

	return 0;
}

int send_syn(in_addr_t our_ipaddr, in_addr_t their_ipaddr, int our_port, int their_port, int seqnr, int sockfd) {
	char* packet = create_packet(our_ipaddr, their_ipaddr, SYN, seqnr, our_port, their_port, "", 0);

	if (send_packet(packet, their_ipaddr, their_port, sockfd) < 0 < 0) {
		perror("Send SYN failed.\n");
		return -1;
	} else {
		// printf("Send SYN.\n");
	}

	return 0;
}

int send_ack(in_addr_t our_ipaddr, in_addr_t their_ipaddr, int our_port, int their_port, int seqnr, int sockfd) {
	char* packet = create_packet(our_ipaddr, their_ipaddr, ACK, seqnr, our_port, their_port, "", 0);

	if (send_packet(packet, their_ipaddr, their_port, sockfd) < 0 < 0) {
		perror("Send ACK failed.\n");
		return -1;
	} else {
		// printf("Send ACK.\n");
	}

	return 0;
}

int send_data(in_addr_t our_ipaddr, in_addr_t their_ipaddr, int our_port, int their_port, int seqnr, char* data, int len, int sockfd) {
	char* packet = create_packet(our_ipaddr, their_ipaddr, NONE, seqnr, our_port, their_port, data, len);

	if (send_packet(packet, their_ipaddr, their_port, sockfd) < 0 < 0) {
		perror("Send data failed.\n");
		return -1;
	} else {
		// printf("Send data.\n");
	}

	return 0;
}
