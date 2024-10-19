#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>

#define MAX_MESSAGE_LENGTH 1024 * 8

const char* port = "8000";
const char* host = "127.0.0.1";
const int backlog = 20;

char* prepareAnswer(char* message, int msgLength, char* host, in_port_t port, int* length) {
  const int portLength = floor(log10(port)) + 1;
  const int hostLength = strlen(host);
  const int totalLength = msgLength + portLength + hostLength + 4;
  char* result = malloc(totalLength);
  sprintf(result, "%s:%d : %s", host, port, message);
  *length = totalLength;
  return result;
}

void* getAddress(const struct sockaddr_storage* sockaddr) {
  if (sockaddr->ss_family == AF_INET) {
    const struct sockaddr_in* ipv4 = (struct sockaddr_in*)sockaddr;
    return (void*)&ipv4->sin_addr;
  }
  const struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)sockaddr;
  return (void*)&ipv6->sin6_addr;
}

in_port_t getPort(const struct sockaddr_storage* sockaddr) {
  in_port_t port;
  if (sockaddr->ss_family == AF_INET) {
    const struct sockaddr_in* ipv4 = (struct sockaddr_in*)sockaddr;
    port = ipv4->sin_port;
  } else {
    const struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)sockaddr;
    port = ipv6->sin6_port;
  }
  return ntohs(port);
}

int getServerSocket(struct addrinfo* ainfo) {
  for (struct addrinfo* info = ainfo; info != NULL; info = info->ai_next) {
    const int descriptor = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    if (descriptor == -1) continue;
    int yes = 1;
    const int setStatus = setsockopt(descriptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
    if (setStatus == -1) continue;
    const int bindStatus = bind(descriptor, info->ai_addr, info->ai_addrlen);
    if (bindStatus == -1) continue;
    return descriptor;
  }
  return -1;
}

int main(const int argc, const char* argv[]) {
  struct addrinfo hints, *result;
  fd_set readfds, master;
  memset(&hints, 0, sizeof hints);
  FD_ZERO(&readfds);
  FD_ZERO(&master);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  const int status = getaddrinfo(host, port, &hints, &result);
  if (status != 0) {
    fprintf(stderr, "Error: %s\n", gai_strerror(status));
    return 1;
  }
  const int serverSocket = getServerSocket(result);
  if (serverSocket == -1) {
    fprintf(stderr, "Error: cannot connect to socket\n");
    return 1;
  }
  const int listenStatus = listen(serverSocket, backlog);
  if (listenStatus == -1) {
    fprintf(stderr, "Error: cannot listen to socket\n");
    return 1;
  }
  printf("Listening: %s:%s\n", host, port);
  FD_SET(serverSocket, &master);
  int maxDescriptor = serverSocket;
  while (1) {
    readfds = master;
    const int selectStatus = select(maxDescriptor + 1, &readfds, NULL, NULL, NULL);
    if (selectStatus == -1) {
      fprintf(stderr, "Error: cannot select\n");
      return 1;
    }
    for (int descriptor = 0; descriptor <= maxDescriptor; descriptor++) {
      if (!FD_ISSET(descriptor, &readfds)) continue;
      if (descriptor == serverSocket) {
        struct sockaddr_storage sockaddr;
        socklen_t socklength = sizeof sockaddr;
        const int clientSocket = accept(serverSocket, (struct sockaddr*)&sockaddr, &socklength);
        if (clientSocket == -1) {
          printf("Error: cannot accept new connectiopn\n");
          continue;
        }
        char ip[INET6_ADDRSTRLEN];
        const void* address = getAddress(&sockaddr);
        inet_ntop(sockaddr.ss_family, address, ip, INET6_ADDRSTRLEN);
        const in_port_t port = getPort(&sockaddr);
        printf("New connectiong from: %s:%d\n", ip, port);
        FD_SET(clientSocket, &master);
        if (clientSocket > maxDescriptor) maxDescriptor = clientSocket;
        break;
      }
      char message[MAX_MESSAGE_LENGTH], ip[INET6_ADDRSTRLEN];
      struct sockaddr_storage sockaddr;
      socklen_t socklength = sizeof sockaddr;
      getpeername(descriptor, (struct sockaddr*)&sockaddr, &socklength);
      const void* address = getAddress(&sockaddr);
      const in_port_t port = getPort(&sockaddr);
      inet_ntop(sockaddr.ss_family, address, ip, INET6_ADDRSTRLEN);
      const int readed = recv(descriptor, message, MAX_MESSAGE_LENGTH, 0);
      if (readed <= 0) {
        printf("Disconnect: %s:%d\n", ip, port);
        FD_CLR(descriptor, &master);
        close(descriptor);
        continue;
      }
      message[readed] = '\0';
      printf("Received message from %s:%d: %s\n", ip, port, message);
      int answerLength = 0;
      char* answer = prepareAnswer(message, readed, ip, port, &answerLength);
      for (int i = 0; i <= maxDescriptor; i++) {
        if (!FD_ISSET(i, &master)) continue;
        if (i == serverSocket || i == descriptor) continue;
        send(i, answer, answerLength, 0);
      }
      free(answer);
    }
  }
  return 0;
}