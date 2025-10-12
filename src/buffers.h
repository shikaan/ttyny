#pragma once

#include "./alloc.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define Buffer(Type)                                                           \
  struct {                                                                     \
    size_t length;                                                             \
    size_t used;                                                               \
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
  result->used = 0;
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

  result->used = length;
  return result;
}

static inline void strDestroy(string_t **self) { deallocate(self); }

static inline void strFmt(string_t *self, const char *fmt, ...) {
  va_list arguments;
  va_start(arguments, fmt);
  int offset = vsnprintf(self->data, self->length + 1, fmt, arguments);
  va_end(arguments);

  if (offset < 0) {
    self->data[0] = 0;
    self->used = 0;
    return;
  }

  if ((size_t)offset > self->length) {
    // Truncated
    self->used = self->length;
  } else {
    self->used = (size_t)offset;
  }
}

static inline void strClear(string_t *self) {
  self->data[0] = 0;
  self->used = 0;
}

#undef makeBufCreate
