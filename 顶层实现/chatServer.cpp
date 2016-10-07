// 使用 Linux 的 API
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void *echo(void *client) {
  // 从 void* 中取出变量
  struct sockaddr_in *clientAddress = (struct sockaddr_in *)((void**)client)[0];
  socklen_t clientAddressSize = sizeof(*clientAddress);
  int *clientSocket = (int *)((void**)client)[1];

  // 获取客户端数据
  getpeername(*clientSocket,
              (struct sockaddr*)clientAddress,
              &clientAddressSize);
  printf("Client: %s\n", inet_ntoa(clientAddress->sin_addr));

  // 从客户端接数据
  int bufferSize = 1024;
  char receiveMessage[bufferSize];
  recv(*clientSocket, receiveMessage, sizeof(receiveMessage), 0);
  printf("Message form client: %s\n", receiveMessage);

  // 向客户端发数据
  char sendMessage[] = "Hello Client!";
  write(*clientSocket, sendMessage, sizeof(sendMessage));

  // 关闭客户端
  close(*clientSocket);
  return ((void*)0);
}

int main() {
  // 创建服务器
  int port = 2014;
  int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  struct sockaddr_in serverAddress;
  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;  // IPv4
  serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");  // 本地服务器
  serverAddress.sin_port = htons(port);  // 端口
  bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
  printf("Server is running on port %d.\n", port);

  // 监听
  while (1) {
    listen(serverSocket, 20);
    // 接收客户端请求
    struct sockaddr_in clientAddress;
    socklen_t clientAddressSize = sizeof(clientAddress);
    int clientSocket = accept(serverSocket,
                              (struct sockaddr*)&clientAddress,
                              &clientAddressSize);
    if (clientSocket) {
      pthread_t tid;
      void *client[2] = {&clientAddress, &clientSocket};
      if ((pthread_create(&tid, NULL, echo, client))!= 0) {
        printf("Can't create thread!\n");
        return 0;
      }
    }
  }

  // 关闭服务器
  close(serverSocket);
  return 0;
}
