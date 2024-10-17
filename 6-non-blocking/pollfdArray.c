#include <malloc.h>
#include <string.h>
#include <poll.h>
#include "pollfdArray.h"

pollfdArray* initpPollfdArray() {
  pollfdArray* array = malloc(sizeof(pollfdArray));
  if (array == NULL) return NULL;
  array->length = 0;
  array->start = NULL;
  return array;
}

int destroyPollfdArray(pollfdArray* array) {
  if (array == NULL) return 0;
  const int length = array->length;
  struct pollfd* start = array->start;
  if (length > 0) free(start);
  free(array);
  return 1;
}

int pushPollfd(pollfdArray* array, struct pollfd item) {
  if (array == NULL) return 0;
  const int length = array->length;
  struct pollfd* pointer = malloc(sizeof(struct pollfd) * (length + 1));
  if (pointer == NULL) return 0;
  if (length > 0) {
    memcpy(pointer, array->start, length * sizeof(struct pollfd*));
    free(array->start);
  }
  pointer[length] = item;
  array->start = pointer;
  array->length += 1;
  return 1;
}

int indexOfPollfd(const pollfdArray* array, const struct pollfd item) {
  struct pollfd* items = array->start;
  for (int i = 0; i < array->length; i++) {
    if (items[i].fd == item.fd) return i;
  }
  return -1;
}

int deletePollfd(pollfdArray* array, const struct pollfd item) {
  if (array == NULL) return 0;
  const int index = indexOfPollfd(array, item);
  if (index == -1) return -1;
  const int length = array->length;
  struct pollfd* start = array->start;
  struct pollfd* pointer = malloc(sizeof(struct pollfd) * (length - 1));
  if (pointer == NULL) return -1;
  memcpy(pointer, start, index * sizeof(struct pollfd*));
  memcpy(pointer + index, start + index + 1, (length - index - 1) * sizeof(struct pollfd*));
  free(start);
  array->start = pointer;
  array->length -= 1;
  return 1;
}

struct pollfd createPollFd(int descriptor, short events) {
  struct pollfd result = { descriptor, events, 0 };
  return result;
}
