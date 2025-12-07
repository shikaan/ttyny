#include "../src/lib/buffers.h"
#include "../src/utils.h"
#include "test.h"
#include <stddef.h>

typedef Buffer(int) test_buffer_t;
static test_buffer_t *testBufferCreate(size_t capacity) {
  test_buffer_t *result;
  bufCreate(test_buffer_t, int, result, capacity);
  return result;
}
static void testBufferDestroy(test_buffer_t **self) { deallocate(self); }
static int testBufferFind(test_buffer_t *self, int item) {
  bufFind(self, item);
}

void stringCreate(void) {
  string_t *subject cleanup(strDestroy) = strCreate(3);
  expectEqls(subject->data, "", 3, "has correct content");
  expectEqllu(subject->cap, 3, "has correct capacity");
  expectEqllu(subject->len, 0, "has correct length");
  strDestroy(&subject);
}

void stringFrom(void) {
  string_t *subject cleanup(strDestroy) = strFrom("lol");
  expectEqls(subject->data, "lol", 3, "has correct content");
  expectEqllu(subject->cap, 3, "has correct capacity");
  expectEqllu(subject->len, 3, "has correct length");
}

void stringFormat(void) {
  string_t *subject cleanup(strDestroy) = strCreate(4);
  strFmt(subject, "%s", "longer");
  expectEqls(subject->data, "long", 4, "truncates content");
  expectEqllu(subject->len, 4, "has correct length");

  strFmt(subject, "%s", "rr");
  expectEqls(subject->data, "rr", subject->cap, "overwrites content");
  expectEqllu(subject->len, 2, "has correct length");
}

void stringClear(void) {
  string_t *subject cleanup(strDestroy) = strFrom("hello");
  size_t capacity = subject->cap;
  strClear(subject);
  expectEqls(subject->data, "", capacity, "truncates content");
  expectEqllu(subject->cap, capacity, "preserves capacity");
  expectEqllu(subject->len, 0, "updates length");
}

void stringTrim(void) {
  string_t *subject cleanup(strDestroy) = strCreate(128);

  strFmt(subject, "%s", "         hello");
  size_t capacity = subject->cap;
  strTrim(subject);
  expectEqls(subject->data, "hello", capacity, "trims left");

  strFmt(subject, "%s", "hello     ");
  strTrim(subject);
  expectEqls(subject->data, "hello", capacity, "trims right");

  strFmt(subject, "%s", "        hello     ");
  strTrim(subject);
  expectEqls(subject->data, "hello", capacity, "trims both");

  strFmt(subject, "%s", "hello");
  strTrim(subject);
  expectEqls(subject->data, "hello", capacity, "trims nothing");

  strFmt(subject, "%s", "he llo");
  strTrim(subject);
  expectEqls(subject->data, "he llo", capacity, "trims nothing (whitespaces)");

  strFmt(subject, "%s", "    ");
  strTrim(subject);
  expectEqls(subject->data, "", capacity, "trims everything");
}

void buffer(void) {
  // out of bounds are expected to panic, they cannot be tested

  test_buffer_t *buf_1 cleanup(testBufferDestroy) = testBufferCreate(2);
  test_buffer_t *buf_2 cleanup(testBufferDestroy) = testBufferCreate(1);
  test_buffer_t *buf_empty cleanup(testBufferDestroy) = testBufferCreate(1);

  case("bufCreate");
  expectEqllu(buf_1->cap, 2, "has correct capacity");
  expectEqllu(buf_1->len, 0, "has correct length");

  case("bufPush");
  panicif(buf_2->len != 0, "unexpected filled buffer");
  bufPush(buf_2, 1);
  expectEqllu(buf_2->len, 1, "increments length");
  bufPush(buf_1, 2);

  case("bufCat");
  bufCat(buf_1, buf_2);
  expectEqllu(buf_1->len, 2, "increases length");
  expectEqli(bufAt(buf_1, 1), 1, "concatenates correctly");
  expectEqllu(buf_2->len, 1, "does not change other buffer");

  bufCat(buf_2, buf_empty);
  expectEqllu(buf_2->len, 1, "pushes an empty list");

  bufCat(buf_empty, buf_2);
  expectEqllu(buf_empty->len, buf_2->len, "pushes to an empty list");

  case("bufRemove");
  size_t initial_len = buf_empty->len;
  bufRemove(buf_empty, bufAt(buf_2, 0), 0);
  expectEqllu(buf_empty->len, initial_len-1, "removes element if found");

  initial_len = buf_empty->len;
  bufRemove(buf_empty, bufAt(buf_2, 0), 0);
  expectEqllu(buf_empty->len, initial_len, "does nothing on empty list");

  initial_len = buf_2->len;
  bufRemove(buf_2, 99, 0);
  expectEqllu(buf_2->len, initial_len, "doesn't change capacity if not found");

  case("bufFind");
  int idx = testBufferFind(buf_empty, 0);
  expectEqli(idx, -1, "returns -1 on empty list");
  idx = testBufferFind(buf_2, 1);
  expectEqli(idx, 0, "returns index of found element");
  idx = testBufferFind(buf_2, 23);
  expectEqli(idx, -1, "returns -1 if element is not found");
}

int main(void) {
  suite(stringCreate);
  suite(stringFrom);
  suite(stringFormat);
  suite(stringClear);
  suite(stringTrim);

  suite(buffer);

  return report();
}
