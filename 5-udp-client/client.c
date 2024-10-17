#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

void* getAddress(const struct sockaddr_storage* sockaddr) {
  if (sockaddr->ss_family == AF_INET) {
    const struct sockaddr_in* ipv4 = (struct sockaddr_in*)sockaddr;
    return (void*)&ipv4->sin_addr;
  }
  const struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)sockaddr;
  return (void*)&ipv6->sin6_addr;
}

int getSocket(struct addrinfo** pinfo) {
  const struct addrinfo* info = *pinfo;
  if (info == NULL) return -1;
  const int descriptor = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
  if (descriptor == -1) {
    *pinfo = (*pinfo)->ai_next;
    return getSocket(pinfo);
  }
  return descriptor;
}

int main(const int argc, const char* argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Error: host and port expected");
    return 1;
  }
  const char* host = argv[1];
  const char* port = argv[2];
  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  const int status = getaddrinfo(host, port, &hints, &result);
  if (status != 0) {
    fprintf(stderr, "Error: %s\n", gai_strerror(status));
    return 1;
  }
  const int socket = getSocket(&result);
  if (socket == -1) {
    fprintf(stderr, "Error: cannot launch server\n");
    return 1;
  }
  const void* address = getAddress((struct sockaddr_storage*)result->ai_addr);
  char ip[INET6_ADDRSTRLEN];
  inet_ntop(result->ai_family, address, ip, INET6_ADDRSTRLEN);
  printf("Message will be sended to %s\n", ip);
  freeaddrinfo(result);
  const char* message = "Hello world!";
  const int sended = sendto(socket, message, strlen(message), 0, result->ai_addr, result->ai_addrlen);
  if (sended == -1) {
    fprintf(stderr, "Error: send message\n");
    return 1;
  }
  close(socket);
  return 0;
}