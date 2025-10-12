#pragma once

#include "./alloc.h"
#include <assert.h>
#include <string.h>

#define Buffer(Type)                                                           \
  struct {                                                                     \
    size_t length;                                                             \
    Type data[];                                                               \
  }

#define bufAt(BufferPtr, Index)                                                \
  {                                                                            \
    assert(Index < BufferPtr->length);                                         \
    BufferPtr->data[Index];                                                    \
  }

#define bufSet(BufferPtr, Index, Value)                                        \
  {                                                                            \
    assert(Index < BufferPtr->length);                                         \
    BufferPtr->data[Index] = Value;                                            \
  }

#define makeBufCreate(BufferType, ItemType, Result, Length)                    \
  const size_t size = sizeof(BufferType);                                      \
  Result = allocate(size + ((unsigned)(Length) * sizeof(ItemType)));           \
  if (!Result) {                                                               \
    return NULL;                                                               \
  }                                                                            \
  Result->length = Length;

typedef Buffer(char) string_t;

static inline string_t *strCreate(size_t length) {
  string_t *result = NULL;
  makeBufCreate(string_t, char, result, length + 1);
  result->length = length;
  result->data[length] = 0;
  return result;
}

static inline string_t *strFrom(const char *initial) {
  size_t length = strlen(initial);
  string_t *result = strCreate(length);
  if (!result) {
    return NULL;
  }

  for (size_t i = 0; i < length; i++) {
    bufSet(result, i, initial[i]);
  }

  return result;
}

static inline void strDestroy(string_t **self) { deallocate(self); }

static inline size_t strWrite(string_t *self, const char *text, size_t offset) {
  if (offset >= self->length)
    return 0;

  size_t text_length = strlen(text);
  size_t available = self->length - offset;
  size_t to_copy = text_length < available ? text_length : available;

  for (size_t i = 0; i < to_copy; ++i) {
    bufSet(self, offset + i, text[i]);
  }

  if (to_copy < available) {
    bufSet(self, offset + to_copy, 0);
  }

  return to_copy;
}

static inline size_t strUsed(const string_t *self) {
  return strlen(self->data);
}

#undef makeBufCreate
