#include <poll.h>

typedef struct {
  int length;
  int fields;
  struct pollfd* start;
} pollfdArray;

pollfdArray* initpPollfdArray();

int destroyPollfdArray(pollfdArray* array);

int pushPollfd(pollfdArray* array, struct pollfd item);

int indexOfPollfd(const pollfdArray* array, const struct pollfd item);

int deletePollfd(pollfdArray* array, const struct pollfd item);

struct pollfd createPollFd(int descriptor, short events);
