// 使用 Linux 的 API
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
// 自己实现的 API
#include "../bottom/common.h"
#include "../bottom/monitor.cpp"
#include "../bottom/mySocket.cpp"
#include "../bottom/net.h"

char onlineClients[16][32];  // 在线客户端的 IP
int numberOfOnlineClients = 0;  // 在线客户端的数量
// 群聊信息
char groupchatMessages[16384];
char peerchatMessages[128][1024];
int numberOfPeerchatMessages = 0;
// peercharMessages 格式形如 127.0.0.1|127.0.0.2: hello
// 第一个 IP 为收方 IP，第二个 IP 为发方 IP，最后是信息内容

void *echo(void *client) {
  // 从 void* 中取出变量
  struct sockaddr_in *clientAddress = (struct sockaddr_in *)((void**)client)[0];
  socklen_t clientAddressSize = sizeof(*clientAddress);
  int *clientSocket = (int *)((void**)client)[1];

  // 从客户端接数据
  int bufferSize = 1024;
  int maxBufferSize = 16384;
  char receiveMessage[bufferSize];
  char receiveBuffer[bufferSize];
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
  } else if (strcmp(receiveMessage, "GroupchatRequest") == 0) {
    // 群聊请求
    write(*clientSocket, groupchatMessages, sizeof(groupchatMessages));
  } else if (strcmp(strncpy(receiveBuffer, receiveMessage, 15), "PeerchatRequest") == 0) {
    int i;
    for (i = 0; i < strlen(receiveMessage) - strlen(receiveBuffer); i++)
      receiveBuffer[i] = receiveMessage[i + 15];
    receiveBuffer[i] = '\0';  // 收方 IP
    char senderIP[32];  // 发方 IP
    strcpy(senderIP, inet_ntoa(clientAddress->sin_addr));
    char tag1[1024], tag2[1024];  // 构造用于查找 peercharMessages 里符合要求的消息
    // 根据标签查找
    strcpy(tag1, strcat(senderIP, "|")); strcat(tag1, receiveBuffer);  // 发方 IP|收方 IP
    strcpy(tag2, strcat(receiveBuffer, "|")); strcat(tag2, senderIP);  // 收方 IP|发方 IP
    char sendMessage[maxBufferSize];
    for (i = 0; i < numberOfPeerchatMessages; i++)
      if (strstr(peerchatMessages[i], tag1) != NULL ||
          strstr(peerchatMessages[i], tag2) != NULL) {
        char *location = strstr(peerchatMessages[i], "|") + 1;
        strcat(sendMessage, location);
        strcat(sendMessage, "+");
      }
    write(*clientSocket, sendMessage, sizeof(sendMessage));
  } else {
    char mode[4]; mode[3] = '\0';
    for (int i = 0; i < 3; i++)
      mode[i] = receiveMessage[i];
    if (strcmp(mode, "|G|") == 0) {
      // 添加发消息用户的 IP
      strcat(groupchatMessages, inet_ntoa(clientAddress->sin_addr));
      strcat(groupchatMessages, ": ");
      // 添加消息内容
      char temp[1024];
      int i;
      for (i = 0; i < sizeof(receiveMessage); i++)
        temp[i] = receiveMessage[i + 3];
      temp[i] = '\0';
      strcat(groupchatMessages, temp);
      // 添加分割符
      strcat(groupchatMessages, "+");
      write(*clientSocket, groupchatMessages, sizeof(groupchatMessages));
    } else if (strcmp(mode, "|P|") == 0) {
      char receiverIP[36];  // 收方 IP
      int i, j;
      for (i = 0; i < 36; i++) {
        if (receiveMessage[i + 3] == '|')
          break;
        receiverIP[i] = receiveMessage[i + 3];
      }
      receiverIP[i] = '\0';
      char senderIP[36];  // 发方 IP
      strcpy(senderIP, inet_ntoa(clientAddress->sin_addr));
      strcpy(peerchatMessages[numberOfPeerchatMessages], receiverIP);
      strcat(peerchatMessages[numberOfPeerchatMessages], "|");
      strcat(peerchatMessages[numberOfPeerchatMessages], senderIP);
      strcat(peerchatMessages[numberOfPeerchatMessages], ": ");
      int start = strlen(peerchatMessages[numberOfPeerchatMessages]);
      for (j = 0; j < strlen(receiveMessage) - i - 3; j++)
        peerchatMessages[numberOfPeerchatMessages][j + start] = receiveMessage[i + j + 4];
      peerchatMessages[numberOfPeerchatMessages][j + start] = '\0';
      numberOfPeerchatMessages++;
      // 写回信息
      char tag1[1024], tag2[1024];  // 构造用于查找 peercharMessages 里符合要求的消息
      // 根据标签查找
      strcpy(tag1, strcat(senderIP, "|")); strcat(tag1, receiverIP);  // 发方 IP|收方 IP
      strcpy(tag2, strcat(receiverIP, "|")); strcat(tag2, senderIP);  // 收方 IP|发方 IP
      char sendMessage[maxBufferSize];
      for (i = 0; i < numberOfPeerchatMessages; i++)
        if (strstr(peerchatMessages[i], tag1) != NULL ||
            strstr(peerchatMessages[i], tag2) != NULL) {
          char *location = strstr(peerchatMessages[i], "|") + 1;
          strcat(sendMessage, location);
          strcat(sendMessage, "+");
        }
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
  groupchatMessages[0] = '\0';
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