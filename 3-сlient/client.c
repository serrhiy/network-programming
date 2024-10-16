#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#define MAX_MESSAGE_LENGTH 1024

const char* port = "8000";

int connectTo(struct addrinfo** pinfo) {
  if (pinfo == NULL) return -1;
  const struct addrinfo* info = *pinfo;
  const int descriptor = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
  if (descriptor == -1) {
    *pinfo = (*pinfo)->ai_next;
    return connectTo(pinfo);
  }
  const int connectStatus = connect(descriptor, info->ai_addr, info->ai_addrlen);
  if (connectStatus == -1) {
    *pinfo = (*pinfo)->ai_next;
    return connectTo(pinfo);
  }
  return descriptor;
}

void* getAddress(struct sockaddr_storage* sockaddr) {
  if (sockaddr->ss_family == AF_INET) {
    const struct sockaddr_in* ipv4 = (struct sockaddr_in*)sockaddr;
    return (void*)&ipv4->sin_addr;
  }
  const struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)sockaddr;
  return (void*)&ipv6->sin6_addr;
}

void printAddress(const struct addrinfo* info, const char* pattern) {
  const void* address = getAddress((struct sockaddr_storage*)info->ai_addr);
  char ip[INET6_ADDRSTRLEN];
  inet_ntop(info->ai_family, address, ip, INET6_ADDRSTRLEN);
  printf(pattern, ip);
}

int main(const int argc, const char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Error: excepts a hostname\n");
    return 1;
  }
  const char* hostname = argv[1];
  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  const int status = getaddrinfo(hostname, port, &hints, &result);
  if (status != 0) {
    fprintf(stderr, "Error: %s\n", hostname);
    return 1;
  }
  const int socket = connectTo(&result);
  if (socket == -1) {
    fprintf(stderr, "Error: cannot connect to socket\n");
    return 1;
  }
  printAddress(result, "Connected to: %s\n");
  freeaddrinfo(result);
  char message[MAX_MESSAGE_LENGTH];
  const int recvStatus = recv(socket, message, MAX_MESSAGE_LENGTH, 0);
  if (recvStatus == -1) {
    fprintf(stderr, "Error: cannot read from socket\n");
    return 1;
  }
  message[recvStatus] = '\0';
  printf("Received message: %s\n", message);
  close(socket);
  return 0;
}