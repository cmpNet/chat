#include "common.h"

int main(int argc, char const *argv[])
{
	int socket = getSocket();
	connect(socket, "127.0.0.1", 10890);
	char message[4096];
	int len = 0;
	read(socket, message, &len);
	char data2[] = "message2";
	write(socket, data2, 8);
	return 0;
}
