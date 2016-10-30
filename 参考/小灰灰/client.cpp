#include "common.h"

int main(int argc, char const *argv[])
{
	int socket = getSocket();
	connect(socket, "127.0.0.1", 10890);
	char message[4096];
	int len = 0;
	read(socket, message, &len);
	for (int i = 0; i < len; i++)
		printf("%c", message[i]);
	printf("\n");
	
	char data[] = "mes2";
	write(socket, data, 8);
	return 0;
}
