#include "common.h"

int main() {
	int socket = getSocket();
	bind(socket, "127.0.0.1");
	listen(socket, 10890);
	int client = accept(socket);
	char data[] = "asasasas";
	write(client, data, 8);
	char buffer[4096];
	int len = 0;
	read(client, buffer, &len);
	return 0;
}
