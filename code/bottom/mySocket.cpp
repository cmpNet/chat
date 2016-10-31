#include "common.h"
#include "net.h"

int init(tcb_t* tcpblock) {
	tcpblock->state = S_CONFIG;
	tcpblock->our_ipaddr = tcpblock->their_ipaddr = 0;
	tcpblock->our_port = tcpblock->their_port = tcpblock->our_seqnr = tcpblock->their_seqnr = tcpblock->acknr = 0;
	memset(tcpblock->buffer, 0, sizeof(tcpblock->buffer));
}

// 成功的话返回socket的id，失败的话返回-1
int getSocket() {
	vector<tcb_t* > *monitor = getMonitor();
	for (int i = 0; i < (*monitor).size(); i++) {
		tcb_t* tcpblock = (*monitor)[i];
		if (tcpblock->state == S_CLOSED) {
			init(tcpblock);
			printf("You uses %d\n", i);
			return i;
		}
	}

	tcb_t* tcpblock = (tcb_t*)malloc(sizeof(tcb_t));
	init(tcpblock);

	monitor->push_back(tcpblock);
	printf("You uses %d\n", monitor->size() - 1);
	return monitor->size() - 1;
}

tcb_t *getTcpBlock(int serverSocket) {
	vector<tcb_t* > *monitor = getMonitor();
	if ((*monitor).size() <= serverSocket) {
		printf("Socket %d is wrong, size: %d\n", serverSocket, (*monitor).size());
		return NULL;
	}
	return (*monitor)[serverSocket];
}

// 成功的话返回1，失败的话返回-1
int bind(int serverSocket, const char ip[]) {
	tcb_t* tcpblock = getTcpBlock(serverSocket);
	if (tcpblock == NULL || tcpblock->state != S_CONFIG)
		return -1;

	tcpblock->our_ipaddr = inet_addr(ip);
	tcpblock->state = S_START;
	return 1;
}

// 成功的话返回1，失败的话返回-1
int listen(int serverSocket, int port) {
	tcb_t* tcpblock = getTcpBlock(serverSocket);
	if (tcpblock == NULL || tcpblock->state != S_START)
		return -1;

	tcpblock->our_port = port;
	tcpblock->state = S_LISTEN;
	return 1;
}

int serverConnection(in_addr_t srcIp, in_addr_t dstIp, int dstPort) {
	static int port_number = 2000;

	int conSocket = getSocket();
	if (conSocket == -1)
		return -1;
	tcb_t* conTcb = getTcpBlock(conSocket);
	if (conTcb == NULL)
		return -1;
	conTcb -> our_ipaddr = srcIp;
	conTcb -> their_ipaddr = dstIp;
	conTcb -> their_port = dstPort;
	//Todo: 记得设置一个随机端口
	conTcb -> our_port = port_number;
	port_number++;
	//Todo: 发送SYN_ACK, 使用my_recv获取ACK包，得到ACK后设置状态为S_ESTABLISHED
	send_syn_ack(srcIp, dstIp, conTcb -> our_port , dstPort, 0);
	my_recv(conTcb);
	struct iphdr* ip = (struct iphdr*)(conTcb->buffer);
	struct tcphdr *tcp = (struct tcphdr *)(conTcb->buffer + sizeof(struct iphdr));
	conTcb -> state = S_ESTABLISHED;

	return conSocket;
}

// 成功的话返回socket的id，失败的话返回-1
int accept(int serverSocket) {
	tcb_t* tcpblock = getTcpBlock(serverSocket);
	if (tcpblock == NULL || tcpblock->state != S_LISTEN)
		return -1;

	while (1) {
		my_recv(tcpblock);
		// printf("Some packet\n");
		struct iphdr* ip = (struct iphdr*)(tcpblock->buffer);
		struct tcphdr *tcp = (struct tcphdr *)(tcpblock->buffer + sizeof(struct iphdr));
		if (/*tcp->ack == 0 &&*/ tcp->syn == 1) {
			return serverConnection(tcpblock->our_ipaddr, ip->saddr, htons(tcp->source));
		}
	}
}

// 成功的话返回1，失败的话返回-1
int connect(int ClientSocket, const char ip_s[], int port) {
	static int port_number = 1000;
	tcb_t* tcpblock = getTcpBlock(ClientSocket);
	if (tcpblock == NULL || tcpblock->state != S_CONFIG)
		return -1;
	// Todo: 获得本机IP，随机设置端口号
	tcpblock -> our_ipaddr = inet_addr("127.0.0.1");
	tcpblock -> our_port = port_number;
	port_number++;

	tcpblock -> their_ipaddr = inet_addr(ip_s);
	tcpblock -> their_port = port;
	// Todo: 发送SYN包，等待SYN_ACK, 然后发送ACK包，设置状态为S_ESTABLISHED
	send_syn(tcpblock -> our_ipaddr, tcpblock -> their_ipaddr, tcpblock -> our_port, tcpblock -> their_port, 0);
	int newPort = tcpblock -> their_port;
	while(1) {
		my_recv(tcpblock);
		struct iphdr* ip = (struct iphdr*)(tcpblock->buffer);
		struct tcphdr *tcp = (struct tcphdr *)(tcpblock->buffer + sizeof(struct iphdr));

		if (tcp->ack == 1 && tcp->syn == 1) {
			newPort = htons(tcp->source);
			break;
		}
	}
	tcpblock -> state = S_ESTABLISHED;
	tcpblock->their_port = newPort;
	int isSuccess = send_ack(tcpblock -> our_ipaddr, tcpblock -> their_ipaddr, tcpblock -> our_port, tcpblock -> their_port, 0);
	return isSuccess;
}

int myRead(int serverSocket, char messageBuffer[], int* bufferLen) {
	// printf("read start\n");
	tcb_t* tcpblock = getTcpBlock(serverSocket);
	if (tcpblock == NULL || tcpblock->state != S_ESTABLISHED)
		return -1;
	printf("read...\n");
	// Todo: 使用my_recv读取数据，发送ACK
	unsigned char* p = (unsigned char*)&(tcpblock -> their_ipaddr);
	//printf("Source IP: %u.%u.%u.%u\n", p[0], p[1], p[2], p[3]);
	//printf("Read Port: %d\n", tcpblock->our_port);
	my_recv(tcpblock);
	printf("received\n");
	//struct iphdr* ip = (struct iphdr*)(tcpblock->buffer);
	struct tcphdr *tcp = (struct tcphdr *)(tcpblock->buffer + sizeof(struct iphdr));
	int data_count = 0;
	char *data = (char*)(tcpblock->buffer + sizeof(struct iphdr) + sizeof(struct tcphdr));
	while (data[data_count] != '\0') {
		messageBuffer[data_count] = data[data_count];
		data_count++;
	}
	// printf("%d\n", data_count);
	*bufferLen = data_count;
	// for (int i = 0; i < data_count; i++)
	// 	 printf("%c", data[i]);
	// printf("\n");
	printf("Send ACK\n");
	return send_ack(tcpblock -> our_ipaddr, tcpblock -> their_ipaddr, tcpblock -> our_port, tcpblock -> their_port, 0);
}

int myWrite(int serverSocket, char message[], int len) {
	tcb_t* tcpblock = getTcpBlock(serverSocket);
	if (tcpblock == NULL || tcpblock->state != S_ESTABLISHED)
		return -1;
	// Todo: 使用send_data发送包, 使用my_recv获取ACK包
	send_data(tcpblock -> our_ipaddr, tcpblock -> their_ipaddr, tcpblock -> our_port, tcpblock -> their_port, 0, message, len);
	unsigned char* p = (unsigned char*)&(tcpblock -> their_ipaddr);
	printf("Source IP: %u.%u.%u.%u\n", p[0], p[1], p[2], p[3]);
	printf("Port: %d\n", tcpblock -> their_port);
	// printf("Data sent\n");
	while(1) {
		my_recv(tcpblock);
		// printf("ACK Received\n");
		struct tcphdr *tcp = (struct tcphdr *)(tcpblock->buffer + sizeof(struct iphdr));
		if (tcp->ack == 1 && tcp->syn == 0)
			break;
	}
	return -1;
}

int myClose(int serverSocket) {
	tcb_t* tcpblock = getTcpBlock(serverSocket);
	if (tcpblock == NULL || tcpblock->state != S_ESTABLISHED)
		return -1;
	tcpblock->state = S_CLOSED;
	//Todo: 四次挥手，之后设置状态为S_CLOSED
	return -1;
}

in_addr_t getPeerIp(int serverSocket) {
	tcb_t* tcpblock = getTcpBlock(serverSocket);
	if (tcpblock == NULL)
		return -1;
	return tcpblock->their_ipaddr;
}
