// 使用 Linux 的 API
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <termios.h>
#include <unistd.h>
// 自己实现的 API
#include "../bottom/common.h"
#include "../bottom/monitor.cpp"
#include "../bottom/mySocket.cpp"
#include "../bottom/net.h"

// 在 Linux 实现 Windows 上的 getch 函数
char getch_() {
  char buf = 0;
  struct termios old = {0};
  if (tcgetattr(0, &old) < 0)
    perror("tcsetattr()");
  old.c_lflag &= ~ICANON;
  old.c_lflag &= ~ECHO;
  old.c_cc[VMIN] = 1;
  old.c_cc[VTIME] = 0;
  if (tcsetattr(0, TCSANOW, &old) < 0)
    perror("tcsetattr ICANON");
  if (read(0, &buf, 1) < 0)
    perror("read()");
  old.c_lflag |= ICANON;
  old.c_lflag |= ECHO;
  if (tcsetattr(0, TCSADRAIN, &old) < 0)
    perror("tcsetattr ~ICANON");
  return buf;
}

bool welcome() {
  system("reset");
  printf("+----------------------------------+\n");
  printf("|       WELCOME TO SOCKETCHAT      |\n");
  printf("+----------------------------------+\n");
  while (1) {
    printf("For signin, enter 'Y'; for exit, enter 'N': ");
    char input[1024];
    scanf("%s", input);
    if ((input[0] == 'Y' || input[0] == 'y') && strlen(input) == 1)
      return 1;
    else if ((input[0] == 'N' || input[0] == 'n') && strlen(input) == 1)
      return 0;
    else
      printf("[ERROR] Invalid input!\n");
  }
}

// 封装的 POST 方法，第一个参数为发往服务器的信息，第二个参数为接受服务器的信息
void post(char *sendMessage, char *receiveMessage) {
  int bufferSize = 1024;
  int port = 2014;
  int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in serverAddress;
  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;  // IPv4
  serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");  // 服务器
  serverAddress.sin_port = htons(port);  // 端口
  connect(clientSocket,
          (struct sockaddr*)&serverAddress,
          sizeof(serverAddress));
  // 向服务器发送数据
  send(clientSocket, sendMessage, strlen(sendMessage), 0);
  // 读回服务器的数据
  read(clientSocket, receiveMessage, bufferSize);
  // 关闭客户端
  close(clientSocket);
}

void signout() {
  int bufferSize = 1024;
  char sendMessage[] = "SignoutRequest";
  char receiveMessage[bufferSize];
  post(sendMessage, receiveMessage);
  if (strcmp(receiveMessage, "Bye") == 0) {
    printf("Sign out successfully!\n");
    exit(0);
  } else {
    printf("[ERROR] Sign out failed! Please retry.\n");
  }
}

void operate();

void printGroup(char* receiveMessage) {
  system("clear");
  printf("+----------------------------------+\n");
  printf("|            GROUP MODE            |\n");
  printf("+----------------------------------+\n");
  printf("Messages:\n");
  const char *d = "+";
  char *p;
  p = strtok(receiveMessage, d);
  while (p) {
    printf("%s\n", p);
    p = strtok(NULL, d);
  }
  printf("\n");
}

char input[1024];
int c, size;

void getUpdate() {
  int maxBufferSize = 16384;
  char receiveMessage[maxBufferSize];
  char sendMessage[] = "GroupchatRequest";
  post(sendMessage, receiveMessage);
  printGroup(receiveMessage);
  printf("Input ('N' to quit, '-' to delete):\n%s", input);
  printf("\n");
}

void getUpdate(char *sendMessage) {
  int maxBufferSize = 16384;
  char receiveMessage[maxBufferSize];
  post(sendMessage, receiveMessage);
  printGroup(receiveMessage);
  printf("Input ('N' to quit, '-' to delete):\n%s", input);
  printf("\n");
}

int stop = 1;

void* updateMessages(void* arg) {
  while (stop) {
    // 更新消息
    getUpdate();
    sleep(1);
  }
  return ((void*)0);
}

void* sendAMessage(void* arg) {
  while (1) {
    c = getch_();
    if (c == '-') {
      size--;
      input[size] = '\0';
      getUpdate();
    } else if (c != '\n') {
      input[size] = c;
      size++;
      input[size] = '\0';
      getUpdate();
    } else if (c == '\n') {
      if (strcmp(input, "N") == 0 || strcmp(input, "n") == 0) {
        stop = 0;
        break;
      }
      char sendMessage[] = "|G|";
      strcat(sendMessage, input);
      getUpdate(sendMessage);
      input[0] = '\0';
      size = 0;
    }
  }
  return ((void*)0);
}

void groupchat() {
  int maxBufferSize = 16384;
  char sendMessage[] = "GroupchatRequest";
  char receiveMessage[maxBufferSize];
  input[0] = '\0';
  size = 0;
  // 加入群聊模式
  post(sendMessage, receiveMessage);
  printGroup(receiveMessage);
  pthread_t update_, send_;
  // 更新消息
  if (pthread_create(&update_, NULL, updateMessages, NULL) != 0) {
    printf("Can't create thread!\n");
    exit(0);
  }
  // 发送消息
  if (pthread_create(&send_, NULL, sendAMessage, NULL) != 0) {
    printf("Can't create thread!\n");
    exit(0);
  }
  pthread_join(update_, NULL);
  pthread_join(send_, NULL);
  signout();
}

void printPeer(char* receiveMessage) {
  system("clear");
  printf("+----------------------------------+\n");
  printf("|             PEER MODE            |\n");
  printf("+----------------------------------+\n");
  printf("Messages:\n");
  const char *d = "+";
  char *p;
  p = strtok(receiveMessage, d);
  while (p) {
    printf("%s\n", p);
    p = strtok(NULL, d);
  }
  printf("\n");
}

void getPeerUpdate() {
  int maxBufferSize = 16384;
  char receiveMessage[maxBufferSize];
  char sendMessage[] = "PeerchatRequest";
  post(sendMessage, receiveMessage);
  printPeer(receiveMessage);
  printf("Input ('N' to quit, '-' to delete):\n%s", input);
  printf("\n");
}

void getPeerUpdate(char *sendMessage) {
  int maxBufferSize = 16384;
  char receiveMessage[maxBufferSize];
  post(sendMessage, receiveMessage);
  printPeer(receiveMessage);
  printf("Input ('N' to quit, '-' to delete):\n%s", input);
  printf("\n");
}

char peer[32];  // 与客户端聊天的 peer 的 IP

void* updatePeerMessages(void* arg) {
  while (stop) {
    // 更新消息
    getPeerUpdate();
    sleep(1);
  }
  return ((void*)0);
}

void* sendAPeerMessage(void* arg) {
  while (1) {
    c = getch_();
    if (c == '-') {
      size--;
      input[size] = '\0';
      getPeerUpdate();
    } else if (c != '\n') {
      input[size] = c;
      size++;
      input[size] = '\0';
      getPeerUpdate();
    } else if (c == '\n') {
      if (strcmp(input, "N") == 0 || strcmp(input, "n") == 0) {
        stop = 0;
        break;
      }
      char sendMessage[] = "|P|";
      strcat(sendMessage, peer);
      strcat(sendMessage, "|");
      strcat(sendMessage, input);
      getPeerUpdate(sendMessage);
      input[0] = '\0';
      size = 0;
    }
  }
  return ((void*)0);
}

void peerchat() {
  printf("Please enter a valid IP:\n");
  scanf("%s", peer);
  stop = 1;
  int bufferSize = 1024;
  char sendMessage[] = "PeerchatRequest";
  strcat(sendMessage, peer);
  char receiveMessage[bufferSize];
  post(sendMessage, receiveMessage);
  printPeer(receiveMessage);
  pthread_t update_, send_;
  // 更新消息
  if (pthread_create(&update_, NULL, updatePeerMessages, NULL) != 0) {
    printf("Can't create thread!\n");
    exit(0);
  }
  // 发送消息
  if (pthread_create(&send_, NULL, sendAPeerMessage, NULL) != 0) {
    printf("Can't create thread!\n");
    exit(0);
  }
  pthread_join(update_, NULL);
  pthread_join(send_, NULL);
  signout();
}

void operate() {
  while (1) {
    printf("+----------------------------------+\n");
    printf("|               HOME               |\n");
    printf("+----------------------------------+\n");
    printf("For groupchat enter 'G';\n");
    printf("For peerchat enter 'P';\n");
    printf("For exit, enter 'Q': ");
    char input[1024];
    scanf("%s", input);
    if ((input[0] == 'Q' || input[0] == 'q') && strlen(input) == 1)
      signout();
    else if ((input[0] == 'P' || input[0] == 'p') && strlen(input) == 1)
      peerchat();
    else if ((input[0] == 'G' || input[0] == 'g') && strlen(input) == 1)
      groupchat();
    else
      printf("[ERROR] Invalid input!\n");
  }
}

void signin() {
  int bufferSize = 1024;
  char sendMessage[] = "SigninRequest";
  char receiveMessage[bufferSize];
  post(sendMessage, receiveMessage);
  printf("\nSign in successfully! The online clients are following:\n");
  printf("+-----------+----------------------+\n");
  printf("| Index     | Address              |\n");
  printf("+-----------+----------------------+\n");
  // 解析数据
  const char *split = "|";
  char *p;
  p = strtok(receiveMessage, split);
  int count = 0;
  while (p != NULL) {
    printf("| %d         | %s", ++count, p);
    for (int i = 0; i < 21 - strlen(p); i++) printf(" ");
    printf("|\n");
    p = strtok(NULL, split);
  }
  printf("+-----------+----------------------+\n");
  operate();
}

int main() {
  if (!welcome())
    return 0;
  // 登录
  printf("- Creating a client service...\n");
  printf("- Receiving message from server...\n");
  printf("- Exit automatically if failed.\n");
  signin();
  return 0;
}
