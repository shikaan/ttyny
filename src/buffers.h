#pragma once

#include "alloc.h"
#include "panic.h"
#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define arrLen(Array) (sizeof(Array) / sizeof(Array[0]))

#define Buffer(Type)                                                           \
  struct {                                                                     \
    size_t length;                                                             \
    size_t used;                                                               \
    Type data[];                                                               \
  }

#define bufInit(Length, Used, ...)                                             \
  {                                                                            \
      (Length),                                                                \
      (Used),                                                                  \
      {__VA_ARGS__},                                                           \
  }

#define bufConst(Length, ...) bufInit(Length, Length, __VA_ARGS__)

#define bufAt(BufferPtr, Index)                                                \
  (panicif((Index) >= (BufferPtr)->length || Index < 0,                        \
           "index out of bounds"),                                             \
   (BufferPtr)->data[(Index)])

#define bufSet(BufferPtr, Index, Value)                                        \
  {                                                                            \
    panicif((Index) >= (BufferPtr)->length || Index < 0,                       \
            "index out of bounds");                                            \
    (BufferPtr)->data[(Index)] = (Value);                                      \
  }

#define bufPush(BufferPtr, Value)                                              \
  {                                                                            \
    bufSet((BufferPtr), (BufferPtr)->used, (Value));                           \
    (BufferPtr)->used++;                                                       \
  }

#define bufClear(BufferPtr, NullValue)                                         \
  {                                                                            \
    bufSet((BufferPtr), 0, (NullValue));                                       \
    (BufferPtr)->used = 0;                                                     \
  }

#define makeBufCreate(BufferType, ItemType, Result, Length)                    \
  const size_t size = sizeof(BufferType);                                      \
  Result = allocate(size + ((unsigned)(Length) * sizeof(ItemType)));           \
  if (!Result) {                                                               \
    return NULL;                                                               \
  }                                                                            \
  Result->length = Length;                                                     \
  Result->used = 0;

// String
typedef Buffer(char) string_t;

#define strConst(String)                                                       \
  {                                                                            \
      .data = String,                                                          \
      .used = sizeof(String) - 1,                                              \
      .length = sizeof(String) - 1,                                            \
  }

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

  result->used = length;
  return result;
}

static inline void strDestroy(string_t **self) { deallocate(self); }

static inline void strFmtOffset(string_t *self, size_t offset, const char *fmt,
                                ...) {
  va_list arguments;
  va_start(arguments, fmt);
  int end_offset =
      vsnprintf(self->data + offset, self->length + 1 - offset, fmt, arguments);
  va_end(arguments);

  if (end_offset < 0) {
    self->data[0] = 0;
    self->used = 0;
    return;
  }

  if ((size_t)end_offset + offset > self->length) {
    // Truncated
    self->used = self->length;
  } else {
    self->used = (size_t)end_offset + offset;
  }
}

#define strFmt(Self, Fmt, ...)                                                 \
  strFmtOffset(Self, 0, Fmt __VA_OPT__(, __VA_ARGS__));

#define strFmtAppend(Self, Fmt, ...)                                           \
  strFmtOffset(Self, (Self)->used, Fmt __VA_OPT__(, __VA_ARGS__));

static inline void strCat(string_t *self, const string_t *other) {
  size_t available = self->length - self->used;
  size_t to_copy = other->used < available ? other->used : available;

  memcpy(self->data + self->used, other->data, to_copy);
  self->used += to_copy;

  self->data[self->used] = 0;
}

static inline int strEq(const string_t *self, const string_t *other) {
  return self->used == other->used &&
         (strncmp(self->data, other->data, self->used) == 0);
}

static inline int strStartsWith(const string_t *haysack,
                                const string_t *needle) {
  return haysack->used >= needle->used &&
         (strncmp(haysack->data, needle->data, needle->used) == 0);
}

static inline void strTrim(string_t *self) {
  size_t begin = 0;
  while (begin < self->used && isspace(bufAt(self, begin)))
    begin++;

  size_t end = self->used - 1;
  while (end > begin && isspace(bufAt(self, end)))
    end--;

  for (size_t i = 0; i < end - begin + 1; i++) {
    bufSet(self, i, bufAt(self, i + begin));
  }

  for (size_t i = end - begin + 1; i < self->used; i++) {
    bufSet(self, i, '\0');
  }

  self->used = end - begin + 1;
}

static inline string_t *strDup(const string_t *self) {
  return strFrom(self->data);
}

static inline void strClear(string_t *self) { bufClear(self, 0); }
