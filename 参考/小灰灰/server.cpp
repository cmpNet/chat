#include "common.h"

int main() {
	int socket = getSocket();
	bind(socket, "127.0.0.1");
	listen(socket, 10890);
	int client = accept(socket);
	char data[] = "mes1";
	write(client, data, 8);

	char message[4096];
	int len = 0;
	read(client, message, &len);
	printf("%d\n", len);
	for (int i = 0; i < len; i++)
		printf("%c", message[i]);
	printf("\n");
	return 0;
}
