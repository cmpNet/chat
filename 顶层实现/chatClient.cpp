#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

bool welcome() {
  printf("=============== WELCOME TO SOCKETCHAT ===============\n");
  while (1) {
    printf("For signin, enter letter 'Y'; for exit, enter letter 'N': ");
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

void signin() {
  // TODO: 改为从服务器取回在线电脑的数组，可考虑类似 JSON 的方式
  struct sockaddr_in serverAddress;
  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;  // IPv4
  serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");  // 服务器
  serverAddress.sin_port = htons(port);  // 端口
  connect(clientSocket,
          (struct sockaddr*)&serverAddress,
          sizeof(serverAddress));

  // 读回服务器的数据
  char buffer[40];
  read(clientSocket, buffer, sizeof(buffer) - 1);
  printf("Message form server: %s\n", buffer);
}

int main() {
  if (!welcome())
    return 0;

  // 创建客户端
  int port = 2014;
  int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

  // 登录
  signin();

  // 关闭客户端
  close(clientSocket);
  return 0;
}
