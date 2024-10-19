#include <malloc.h>
#include <string.h>
#include <poll.h>
#include "pollfdArray.h"

const int ARRAY_FIELDS = 3;

pollfdArray* initpPollfdArray() {
  pollfdArray* array = malloc(sizeof(pollfdArray));
  if (array == NULL) return NULL;
  array->start = malloc(sizeof(struct pollfd) * ARRAY_FIELDS);
  if (array->start == NULL) {
    free(array);
    return NULL;
  }
  array->length = 0;
  array->fields = ARRAY_FIELDS;
  return array;
}

int destroyPollfdArray(pollfdArray* array) {
  if (array == NULL) return 0;
  free(array->start);
  free(array);
  return 1;
}

int pushPollfd(pollfdArray* array, struct pollfd item) {
  if (array == NULL) return 0;
  const int length = array->length;
  const int fields = array->fields;
  struct pollfd* start = array->start;
  if (length < fields) {
    array->length++;
    start[length] = item;
    return 1;
  }
  const int newFields = fields + ARRAY_FIELDS;
  struct pollfd* pointer = malloc(sizeof(struct pollfd) * newFields);
  if (pointer == NULL) return 0;
  memcpy(pointer, array->start, fields * sizeof(struct pollfd));
  free(array->start);
  pointer[length] = item;
  array->start = pointer;
  array->length++;
  array->fields = newFields;
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
  if (index == -1) return 0;
  const int length = array->length;
  const int fields = array->fields;
  struct pollfd* start = array->start;
  if (fields - length >= ARRAY_FIELDS) {
    const int newFields = fields - ARRAY_FIELDS;
    struct pollfd* pointer = malloc(sizeof(struct pollfd) * newFields);
    if (pointer == NULL) return 0;
    memcpy(pointer, start, index * sizeof(struct pollfd));
    memcpy(pointer + index, start + index + 1, (length - index - 1) * sizeof(struct pollfd));
    free(start);
    array->start = pointer;
    array->length--;
    array->fields = newFields;
    return 1;
  }
  memmove(start + index, start + index + 1, (length - index - 1) * sizeof(struct pollfd));
  array->length--;
  return 1;
}

struct pollfd createPollFd(int descriptor, short events) {
  struct pollfd result = { descriptor, events, 0 };
  return result;
}
