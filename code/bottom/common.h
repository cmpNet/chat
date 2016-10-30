#ifndef COMMON
#define COMMON

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
#include <vector>
using std::vector;

#include <arpa/inet.h>
#include <linux/if_packet.h>
//#include <linux/ip.h>
//#include <linux/tcp.h>
//#include <linux/udp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
//#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <unistd.h>

#define PACKETLEN 1024 * 4 

typedef enum {
	FIN, SYN, RST, PSH, ACK, SYN_ACK, URG, NONE
}FLAG;

typedef enum {
    S_START, S_CLOSED, S_CONNECTING, S_LISTEN, S_SYN_SENT, S_SYN_ACK_SENT,
    S_SYN_RECEIVED, S_ESTABLISHED, S_FIN_WAIT_1, S_FIN_WAIT_2, S_CLOSE_WAIT,
    S_TIME_WAIT, S_CLOSING, S_LAST_ACK, S_CONFIG
} state_t;

typedef struct tcb {
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

vector<tcb_t* > *getMonitor();

// 成功的话返回socket的id，失败的话返回-1
int getSocket();

// 成功的话返回1，失败的话返回-1
int bind(int serverSocket, const char ip[]);
// 成功的话返回1，失败的话返回-1
int listen(int serverSocket, int port);

// 成功的话返回socket的id，失败的话返回-1
int accept(int serverSocket);

// 成功的话返回1，失败的话返回-1
int connect(int socket, const char ip[], int port);

// 成功的话返回1，失败的话返回-1
int read(int socket, char messageBuffer[], int* bufferLen);
// 成功的话返回1，失败的话返回-1
int write(int socket, char message[], int len);
// 成功的话返回1，失败的话返回-1
int close(int socket);

#endif
