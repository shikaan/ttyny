#pragma once

#include "./alloc.h"
#include <assert.h>

#define Buffer(Type)                                                           \
  struct {                                                                     \
    int length;                                                                \
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
  const int size = sizeof(BufferType);                                         \
  Result = allocate(size + ((unsigned)(Length) * sizeof(ItemType)));           \
  if (!Result) {                                                               \
    return NULL;                                                               \
  }                                                                            \
  Result->length = Length;

typedef Buffer(char) string_t;

static inline string_t *strCreate(int length) {
  string_t *result = NULL;
  makeBufCreate(string_t, char, result, length + 1);
  result->length = length;
  result->data[length] = 0;
  return result;
}

static inline string_t *strFrom(const char *initial) {
  int length = 0;
  for (; initial[length] != 0; length++)
    ;

  string_t *result = strCreate(length);
  if (!result) {
    return NULL;
  }

  for (int i = 0; i < length; i++) {
    bufSet(result, i, initial[i]);
  }

  return result;
}

static inline void strDestroy(string_t **self) { deallocate(self); }

#undef makeBufCreate
