#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>\

#define MAX_MESSAGE_SIZE 8192

const char* port = "8000";
const char* host = "127.0.0.1";

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
  const int bindStatus = bind(descriptor, info->ai_addr, info->ai_addrlen);
  if (bindStatus == -1) return getSocket(info->ai_next);
  return descriptor;
}

int main(const int argc, const char* argv[]) {
  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  const int status = getaddrinfo(host, port, &hints, &result);
  if (status != 0) {
    fprintf(stderr, "Error: %s\n", gai_strerror(status));
    return 1;
  }
  const int socket = getSocket(result);
  freeaddrinfo(result);
  if (socket == -1) {
    fprintf(stderr, "Error: cannot connect to socket\n");
    return 1;
  }
  printf("server: waiting for connections...\n");
  while (1) {
    char message[MAX_MESSAGE_SIZE], ip[INET6_ADDRSTRLEN];
    struct sockaddr_storage sockStorage;
    socklen_t socklength = sizeof sockStorage;
    const int readed = recvfrom(socket, message, MAX_MESSAGE_SIZE, 0, (struct sockaddr*)&sockStorage, &socklength);
    if (readed == -1) {
      fprintf(stderr, "Error: cannot get message\n");
      continue;
    }
    message[readed] = '\0';
    const void* address = getAddress(&sockStorage);
    inet_ntop(sockStorage.ss_family, address, ip, INET6_ADDRSTRLEN);
    printf("Message from: %s: %s\n", ip, message);
  }
  close(socket);
  return 0;
}
