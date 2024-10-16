#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

const char* port = "8000";
const int backlog = 10;

void* getAddress(const struct sockaddr_storage* sockaddr) {
  if (sockaddr->ss_family == AF_INET) {
    const struct sockaddr_in* ipv4 = (struct sockaddr_in*)sockaddr;
    return (void*)&ipv4->sin_addr;
  }
  const struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)sockaddr;
  return (void*)&ipv6->sin6_addr;
}

int getSocket(const struct addrinfo* info) {
  if (info == NULL) return -1;
  const int descriptor = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
  if (descriptor == -1) return getSocket(info->ai_next);
  int yes = 1;
  const int optionsStatus = setsockopt(descriptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  if (optionsStatus == -1) return getSocket(info->ai_next);
  const int bindStatus = bind(descriptor, info->ai_addr, info->ai_addrlen);
  if (bindStatus == -1) return getSocket(info->ai_next);
  return descriptor;
}

int main(const int argc, const char* argv[]) {
  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof hints);
  // hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  const int success = getaddrinfo("127.0.0.1", port, &hints, &result);
  if (success != 0) {
    fprintf(stderr, "Error: %s\n",gai_strerror(success));
    return 1; 
  }
  const int socket = getSocket(result);
  freeaddrinfo(result);
  if (socket == -1) {
    fprintf(stderr, "Error: cannot bind to socket\n");
    return 1;
  }
  if (listen(socket, backlog) == -1) {
    fprintf(stderr, "Error: cannot listen to port\n");
    return 1;
  }
  printf("server: waiting for connections...\n");
  while (1) {
    struct sockaddr_storage sockaddr;
    socklen_t socklength = sizeof sockaddr;
    char ip[INET6_ADDRSTRLEN];
    const int clientDescriptor = accept(socket, (struct sockaddr*)&sockaddr, &socklength);
    if (clientDescriptor == -1) {
      printf("Error whyle accepting");
      continue;
    };
    const void* address = getAddress(&sockaddr);
    inet_ntop(sockaddr.ss_family, address, ip, INET6_ADDRSTRLEN);
    printf("Request from: %s\n", ip);
    const int sendId = send(clientDescriptor, "Hello world!", 12, 0);
    printf("Send: %d\n", sendId);
    close(clientDescriptor);
  }
  return 0;
}
