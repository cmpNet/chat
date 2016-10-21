// 使用 Linux 的 API
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <string>
using namespace std;

char onlineClients[16][32];  // 在线客户端的 IP
int numberOfOnlineClients = 0;  // 在线客户端的数量
enum MODE {GROUP, PEER, COMMAND};
int mode = COMMAND;
vector<string> groupChatMessages;

void *echo(void *client) {
  // 从 void* 中取出变量
  struct sockaddr_in *clientAddress = (struct sockaddr_in *)((void**)client)[0];
  socklen_t clientAddressSize = sizeof(*clientAddress);
  int *clientSocket = (int *)((void**)client)[1];

  // 从客户端接数据
  int bufferSize = 1024;
  char receiveMessage[bufferSize];
  recv(*clientSocket, receiveMessage, sizeof(receiveMessage), 0);

  // 调试输出
  printf("Message form client: %s\n", receiveMessage);

  // 处理信息
  if (strcmp(receiveMessage, "SigninRequest") == 0) {  // 登录请求
    // 获取客户端的 IP
    getpeername(*clientSocket,
                (struct sockaddr*)clientAddress,
                &clientAddressSize);
    int index = numberOfOnlineClients;
    strcpy(onlineClients[index], inet_ntoa(clientAddress->sin_addr));
    numberOfOnlineClients++;
    // 向客户端发数据
    char sendMessage[bufferSize];
    int offset = 0;
    for (int i = 0; i < numberOfOnlineClients; i++) {
      if (i != numberOfOnlineClients - 1)
        offset += sprintf(sendMessage + offset, "%s|", onlineClients[i]);
      else
        sprintf(sendMessage + offset, "%s", onlineClients[i]);
    }
    write(*clientSocket, sendMessage, sizeof(sendMessage));
  } else if (strcmp(receiveMessage, "SignoutRequest") == 0) {
    // 获取客户端的 IP
    getpeername(*clientSocket,
                (struct sockaddr*)clientAddress,
                &clientAddressSize);
    int index = numberOfOnlineClients;
    for (int i = 0; i < index; i++)
      if (strcmp(onlineClients[i], inet_ntoa(clientAddress->sin_addr)) == 0) {
        for (int j = i; j < index - 1; j++)
          strcmp(onlineClients[j], onlineClients[j + 1]);
        index--;
      }
    // 向客户端发数据
    char sendMessage[] = "Bye";
    write(*clientSocket, sendMessage, sizeof(sendMessage));
  } else if (strcmp(receiveMessage, "GroupchatRequest") == 0) { //  群聊请求
    mode = GROUP;
    char sendMessage[] = "GROUP mode";
    write(*clientSocket, sendMessage, sizeof(sendMessage));
  } else if (strcmp(receiveMessage, "CHECK") == 0) { //  查看消息
    if (mode == GROUP) { //  查看群聊消息
      string sendMessage = "";
      if (groupChatMessages.size() > 0) {
        sendMessage += groupChatMessages[0];
      }
      for (int i = 1; i < groupChatMessages.size(); ++i) {
        sendMessage += "\n" + groupChatMessages[i];
      }
      write(*clientSocket, sendMessage.c_str(), sendMessage.length());
    }
  } else if (strcmp(receiveMessage, "EXIT") == 0) { //  退出聊天
    if (mode == GROUP) { // 退出群聊
      mode = COMMAND;
      char sendMessage[] = "EXIT";
      write(*clientSocket, sendMessage, sizeof(sendMessage));
    }
  } else { //  接收客户端消息
    if (mode == GROUP) {
      string ip(inet_ntoa(clientAddress->sin_addr));
      string msg(receiveMessage);
      groupChatMessages.push_back(ip + ": " + msg);
      char sendMessage[] = "Server: Receive the message";
      write(*clientSocket, sendMessage, sizeof(sendMessage));
    }
  }

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
