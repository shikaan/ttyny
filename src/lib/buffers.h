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
    size_t cap;                                                                \
    size_t len;                                                                \
    Type data[];                                                               \
  }

#define bufInit(Cap, Len, ...)                                                 \
  {                                                                            \
      (Cap),                                                                   \
      (Len),                                                                   \
      {__VA_ARGS__},                                                           \
  }

#define bufConst(Cap, ...) bufInit(Cap, Cap, __VA_ARGS__)

#define bufAt(BufferPtr, Index)                                                \
  (panicif((Index) >= (BufferPtr)->cap || Index < 0, "index out of bounds"),   \
   (BufferPtr)->data[(Index)])

#define bufSet(BufferPtr, Index, Value)                                        \
  {                                                                            \
    panicif((Index) >= (BufferPtr)->cap || Index < 0, "index out of bounds");  \
    (BufferPtr)->data[(Index)] = (Value);                                      \
  }

#define bufPush(BufferPtr, Value)                                              \
  {                                                                            \
    bufSet((BufferPtr), (BufferPtr)->len, (Value));                            \
    (BufferPtr)->len++;                                                        \
  }

#define bufClear(BufferPtr, NullValue)                                         \
  {                                                                            \
    bufSet((BufferPtr), (size_t)0, (NullValue));                               \
    (BufferPtr)->len = 0;                                                      \
  }

#define bufCreate(BufferType, ItemType, Result, Cap)                           \
  const size_t size = sizeof(BufferType);                                      \
  Result = allocate(size + ((unsigned)(Cap) * sizeof(ItemType)));              \
  if (!Result) {                                                               \
    return NULL;                                                               \
  }                                                                            \
  Result->cap = Cap;                                                           \
  Result->len = 0;

#define bufEach(BufferPtr, Index) for ((Index) = 0; i < (BufferPtr)->len; i++)

#define bufCat(BufferPtr, OtherBufferPtr)                                      \
  {                                                                            \
    size_t i;                                                                  \
    bufEach(OtherBufferPtr, i) {                                               \
      bufPush((BufferPtr), bufAt((OtherBufferPtr), i));                        \
    }                                                                          \
  }

#define bufRemove(BufferPtr, Item, NullValue)                                  \
  {                                                                            \
    size_t i;                                                                  \
    bufEach(BufferPtr, i) {                                                    \
      if (bufAt(BufferPtr, i) == Item) {                                       \
        bufSet(BufferPtr, i, bufAt(BufferPtr, (BufferPtr)->len - 1));          \
        bufSet(BufferPtr, (BufferPtr)->len - 1, (NullValue));                  \
        (BufferPtr)->len--;                                                    \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
  }

#define bufFind(BufferPtr, Item)                                               \
  {                                                                            \
    size_t i;                                                                  \
    bufEach(BufferPtr, i) {                                                    \
      if (bufAt(BufferPtr, i) == Item) {                                       \
        return (int)i;                                                         \
      }                                                                        \
    }                                                                          \
    return -1;                                                                 \
  }

#define bufIsEmpty(BufferPtr) (!(BufferPtr)->len)

// String
typedef Buffer(char) string_t;

#define strConst(String)                                                       \
  {                                                                            \
      .data = String,                                                          \
      .len = sizeof(String) - 1,                                               \
      .cap = sizeof(String) - 1,                                               \
  }

static inline string_t *strCreate(size_t cap) {
  string_t *result = NULL;
  bufCreate(string_t, char, result, cap + 1);
  result->cap = cap;
  result->data[cap] = 0;
  return result;
}

static inline string_t *strFrom(const char *initial) {
  size_t cap = strlen(initial);
  string_t *result = strCreate(cap);
  if (!result) {
    return NULL;
  }

  for (size_t i = 0; i < cap; i++) {
    bufSet(result, i, initial[i]);
  }

  result->len = cap;
  return result;
}

static inline void strDestroy(string_t **self) { deallocate(self); }

static inline void strFmtOffset(string_t *self, size_t offset, const char *fmt,
                                ...) {
  va_list arguments;
  va_start(arguments, fmt);
  int end_offset =
      vsnprintf(self->data + offset, self->cap + 1 - offset, fmt, arguments);
  va_end(arguments);

  if (end_offset < 0) {
    self->data[0] = 0;
    self->len = 0;
    return;
  }

  if ((size_t)end_offset + offset > self->cap) {
    // Truncated
    self->len = self->cap;
  } else {
    self->len = (size_t)end_offset + offset;
  }
}

#define strFmt(Self, Fmt, ...)                                                 \
  strFmtOffset(Self, 0, Fmt __VA_OPT__(, __VA_ARGS__));

#define strFmtAppend(Self, Fmt, ...)                                           \
  strFmtOffset(Self, (Self)->len, Fmt __VA_OPT__(, __VA_ARGS__));

static inline void strCat(string_t *self, const string_t *other) {
  size_t available = self->cap - self->len;
  size_t to_copy = other->len < available ? other->len : available;

  memcpy(self->data + self->len, other->data, to_copy);
  self->len += to_copy;

  self->data[self->len] = 0;
}

static inline int strEq(const string_t *self, const string_t *other) {
  return self->len == other->len &&
         (strncmp(self->data, other->data, self->len) == 0);
}

static inline int strStartsWith(const string_t *haysack,
                                const string_t *needle) {
  return haysack->len >= needle->len &&
         (strncmp(haysack->data, needle->data, needle->len) == 0);
}

static inline void strTrim(string_t *self) {
  size_t begin = 0;
  while (begin < self->len && isspace(bufAt(self, begin)))
    begin++;

  size_t end = self->len - 1;
  while (end > begin && isspace(bufAt(self, end)))
    end--;

  for (size_t i = 0; i < end - begin + 1; i++) {
    bufSet(self, i, bufAt(self, i + begin));
  }

  for (size_t i = end - begin + 1; i < self->len; i++) {
    bufSet(self, i, '\0');
  }

  self->len = end - begin + 1;
}

static inline string_t *strDup(const string_t *self) {
  return strFrom(self->data);
}

// If failing, returns pointer to first failing replacement - else null
static inline char *strCaseReplace(string_t *self, const char *from,
                               const char *to) {
  if (!from || !to)
    return NULL;

  size_t from_len = strlen(from);
  size_t to_len = strlen(to);

  if (from_len == 0)
    return NULL;

  char *token;
  char *search_start = self->data;
  while ((token = strcasestr(search_start, from)) != NULL) {
    if (from_len == to_len) {
      memmove(token, to, from_len);
      search_start = token + to_len;
      continue;
    }

    size_t tok_len = strlen(token);

    size_t delta = to_len - from_len;
    if (from_len < to_len) {
      if (self->len + delta >= self->cap) {
        return token;
      }

      memmove(token + to_len, token + from_len, tok_len - from_len + 1);
      memmove(token, to, to_len);
    } else {
      memmove(token, to, to_len);
      memmove(token + to_len, token + from_len, tok_len - from_len + 1);
    }
    self->len += delta;

    search_start = token + to_len;
  }

  return token;
}

static inline void strClear(string_t *self) { bufClear(self, 0); }

typedef Buffer(string_t *) strings_t;
