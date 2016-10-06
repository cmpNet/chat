// 使用 Linux 的 API
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  // 端口号由参数提供
  if (argc != 2) {
      printf("No port provided!\n");
      return 0;
  }

  // 创建服务器
  int port = atoi(argv[1]);
  int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  struct sockaddr_in serverAddress;
  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;  // IPv4
  serverAddress.sin_addr.s_addr = inet_addr("localhost");  // 本地服务器
  serverAddress.sin_port = htons(port);  // 端口
  bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
  printf("Server is running on port %d.\n", port);

  return 0;
}
