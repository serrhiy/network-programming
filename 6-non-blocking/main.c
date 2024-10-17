#include <stdio.h>
#include "pollfdArray.h"

void print(const pollfdArray* array) {
  for (int i = 0; i < array->length; i++) {
    const struct pollfd* item = array->start[i];
    printf("descriptor: %d\t events: %d\t revetns: %d\n", item->fd, item->events, item->revents);
  }
}

int main(const int argc, const char* argv[]) {
  pollfdArray* array = initpPollfdArray();
  struct pollfd* fd = createPollFd(32, 0, 0);
  pushPollfd(array, createPollFd(30, 0, 0));
  pushPollfd(array, createPollFd(31, 0, 0));
  pushPollfd(array, fd);
  printf("Length: %d\n", array->length);
  print(array);
  deletePollfd(array, fd);
  printf("Length: %d\n", array->length);
  print(array);
  return 0;
}