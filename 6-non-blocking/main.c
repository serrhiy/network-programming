#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "pollfdArray.h"

#define MAX_MESSAGE_LENGTH 1024 * 8

const char* port = "8000";
const char* host = "192.168.31.112";
const int backlog = 20;

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

int getServerSocket(const struct addrinfo* info) {
  if (info == NULL) return -1;
  const int descriptor = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
  if (descriptor == -1) return getServerSocket(info->ai_next);
  int yes = 1;
  const int optionsStatus = setsockopt(descriptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
  if (optionsStatus == -1) return getServerSocket(info->ai_next);
  const int bindStatus = bind(descriptor, info->ai_addr, info->ai_addrlen);
  if (bindStatus == -1) return getServerSocket(info->ai_next);
  return descriptor;
}

char* prepareAnswer(char* message, int msgLength, char* host, in_port_t port, int* length) {
  const int portLength = floor(log10(port)) + 1;
  const int hostLength = strlen(host);
  const int totalLength = msgLength + portLength + hostLength + 4;
  char* result = malloc(totalLength);
  sprintf(result, "%s:%d : %s", host, port, message);
  *length = totalLength;
  return result;
}

int main(const int argc, const char* argv[]) {
  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  const int status = getaddrinfo(host, port, &hints, &result);
  if (status != 0) {
    fprintf(stderr, "Error: %s\n", gai_strerror(status));
    return -1;
  }
  const int serverSocket = getServerSocket(result);
  if (serverSocket == -1) {
    fprintf(stderr, "Error: cannot create server socket\n");
    return -1;
  }
  const int listenStatus = listen(serverSocket, backlog);
  if (listenStatus == -1) {
    close(serverSocket);
    fprintf(stderr, "Error: cannot listen server socket\n");
    return -1;
  }
  printf("Listening: %s:%s\n", host, port);
  pollfdArray* sockets = initpPollfdArray();
  pushPollfd(sockets, createPollFd(serverSocket, POLLIN));
  while (1) {
    const int eventsCount = poll(sockets->start, sockets->length, -1);
    if (eventsCount == -1) {
      fprintf(stderr, "Error: cannot poll events\n");
      return -1;
    }
    for (int i = 0; i < sockets->length; i++) {
      const struct pollfd socket = sockets->start[i];
      if (!(socket.revents & POLLIN)) continue;
      if (socket.fd == serverSocket) {
        struct sockaddr_storage sockaddr;
        socklen_t socklength = sizeof sockaddr;
        const int clientSocket = accept(serverSocket, (struct sockaddr*)&sockaddr, &socklength);
        if (clientSocket == -1) {
          printf("Error: cannot accept new connectiopn\n");
          continue;
        }
        const void* address = getAddress(&sockaddr);
        const in_port_t port = getPort(&sockaddr);
        char ip[INET6_ADDRSTRLEN];
        inet_ntop(sockaddr.ss_family, address, ip, INET6_ADDRSTRLEN);
        printf("New connectiong from: %s:%d\n", ip, port);
        pushPollfd(sockets, createPollFd(clientSocket, POLLIN));
        continue;
      }
      char message[MAX_MESSAGE_LENGTH], ip[INET6_ADDRSTRLEN];
      struct sockaddr_storage sockaddr;
      socklen_t socklength = sizeof sockaddr;
      getpeername(socket.fd, (struct sockaddr*)&sockaddr, &socklength);
      const void* address = getAddress(&sockaddr);
      const in_port_t port = getPort(&sockaddr);
      inet_ntop(sockaddr.ss_family, address, ip, INET6_ADDRSTRLEN);
      const int readed = recv(socket.fd, message, MAX_MESSAGE_LENGTH, 0);
      if (readed <= 0) {
        printf("Disconnect: %s:%d\n", ip, port);
        deletePollfd(sockets, socket);
        close(socket.fd);
        continue;
      }
      message[readed] = '\0';
      printf("Received message from %s:%d: %s\n", ip, port, message);
      int answerLength = 0;
      char* answer = prepareAnswer(message, readed, ip, port, &answerLength);
      for (int i = 0; i < sockets->length; i++) {
        const struct pollfd connection = sockets->start[i];
        if (connection.fd == socket.fd || connection.fd == serverSocket) continue;
        send(connection.fd, answer, answerLength, 0);
      }
      free(answer);
    }
  }
  return 0;
}
