#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(const int argc, const char* argv[]) {
  if (argc != 2) {
    printf("error: need hostname");
    return 1;
  }
  const char* hostname = argv[1];
  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  const int status = getaddrinfo(hostname, NULL, &hints, &result);
  if (status != 0) {
    printf("error: %s\n", gai_strerror(status));
    return 1;
  }
  printf("IP adress for: %s\n", hostname);
  for (struct addrinfo* info = result; info != NULL; info = info->ai_next) {
    char const* ipver;
    void* addr;
    char ip[INET6_ADDRSTRLEN];
    if (info->ai_family == AF_INET) {
      const struct sockaddr_in* ipv4 = (struct sockaddr_in*)info->ai_addr;
      addr = (void*)&ipv4->sin_addr;
      ipver = "IPv4";
    } else {
      const struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)info->ai_addr;
      addr = (void*)&ipv6->sin6_addr;
      ipver = "IPv6";
    }
    inet_ntop(info->ai_family, addr, ip, INET6_ADDRSTRLEN);
    printf("version: %s\t ip: %s\n", ipver, ip);
  }
}
