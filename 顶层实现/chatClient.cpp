#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

bool welcome() {
  system("reset");
  printf("+----------------------------------------------------+\n");
  printf("|                WELCOME TO SOCKETCHAT               |\n");
  printf("+----------------------------------------------------+\n");
  while (1) {
    printf("For signin, enter 'Y'; for exit, enter 'N': ");
    size_t maxSize = 2048;
    char *input = (char*)malloc(maxSize);
    getline(&input, &maxSize, stdin);
    if ((input[0] == 'Y' || input[0] == 'y') && strlen(input) == 2) {
      free(input);
      return 1;
    } else if ((input[0] == 'N' || input[0] == 'n') && strlen(input) == 2) {
      free(input);
      return 0;
    } else {
      free(input);
      printf("Invalid input!\n");
    }
  }
}

void signin(int port, int clientSocket) {
  struct sockaddr_in serverAddress;
  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;  // IPv4
  serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");  // 服务器
  serverAddress.sin_port = htons(port);  // 端口
  connect(clientSocket,
          (struct sockaddr*)&serverAddress,
          sizeof(serverAddress));

  // 向服务器发送数据
  int bufferSize = 1024;
  char sendMessage[] = "Hello server!";
  send(clientSocket, sendMessage, sizeof(sendMessage), 0);

  // 读回服务器的数据
  char receiveMessage[bufferSize];
  read(clientSocket, receiveMessage, sizeof(receiveMessage) - 1);
  printf("Message form server: %s\n", receiveMessage);
}

int main() {
  if (!welcome())
    return 0;

  // 创建客户端
  printf("Creating a client service...\n");
  int port = 2014;
  int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

  // 登录
  printf("Receiving message from server...\n");
  signin(port, clientSocket);

  // 关闭客户端
  close(clientSocket);
  return 0;
}
