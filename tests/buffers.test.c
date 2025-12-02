#include "../src/lib/buffers.h"
#include "../src/utils.h"
#include "test.h"
#include <stddef.h>

typedef Buffer(int) test_buffer_t;
static test_buffer_t *testBufferCreate(size_t length) {
  test_buffer_t *result;
  bufCreate(test_buffer_t, int, result, length);
  return result;
}
static void testBufferDestroy(test_buffer_t **self) { deallocate(self); }
static int testBufferFind(test_buffer_t *self, int item) {
  bufFind(self, item);
}

void stringCreate(void) {
  string_t *subject cleanup(strDestroy) = strCreate(3);
  expectEqls(subject->data, "", 3, "has correct content");
  expectEqllu(subject->length, 3, "has correct length");
  expectEqllu(subject->used, 0, "has correct used");
  strDestroy(&subject);
}

void stringFrom(void) {
  string_t *subject cleanup(strDestroy) = strFrom("lol");
  expectEqls(subject->data, "lol", 3, "has correct content");
  expectEqllu(subject->length, 3, "has correct length");
  expectEqllu(subject->used, 3, "has correct used");
}

void stringFormat(void) {
  string_t *subject cleanup(strDestroy) = strCreate(4);
  strFmt(subject, "%s", "longer");
  expectEqls(subject->data, "long", 4, "truncates content");
  expectEqllu(subject->used, 4, "has correct used");

  strFmt(subject, "%s", "rr");
  expectEqls(subject->data, "rr", subject->length, "overwrites content");
  expectEqllu(subject->used, 2, "has correct used");
}

void stringClear(void) {
  string_t *subject cleanup(strDestroy) = strFrom("hello");
  size_t length = subject->length;
  strClear(subject);
  expectEqls(subject->data, "", length, "truncates content");
  expectEqllu(subject->length, length, "preserves length");
  expectEqllu(subject->used, 0, "updates used");
}

void stringTrim(void) {
  string_t *subject cleanup(strDestroy) = strCreate(128);

  strFmt(subject, "%s", "         hello");
  size_t length = subject->length;
  strTrim(subject);
  expectEqls(subject->data, "hello", length, "trims left");

  strFmt(subject, "%s", "hello     ");
  strTrim(subject);
  expectEqls(subject->data, "hello", length, "trims right");

  strFmt(subject, "%s", "        hello     ");
  strTrim(subject);
  expectEqls(subject->data, "hello", length, "trims both");

  strFmt(subject, "%s", "hello");
  strTrim(subject);
  expectEqls(subject->data, "hello", length, "trims nothing");

  strFmt(subject, "%s", "he llo");
  strTrim(subject);
  expectEqls(subject->data, "he llo", length, "trims nothing (whitespaces)");

  strFmt(subject, "%s", "    ");
  strTrim(subject);
  expectEqls(subject->data, "", length, "trims everything");
}

void buffer(void) {
  // out of bounds are expected to panic, they cannot be tested

  test_buffer_t *buf_1 cleanup(testBufferDestroy) = testBufferCreate(2);
  test_buffer_t *buf_2 cleanup(testBufferDestroy) = testBufferCreate(1);
  test_buffer_t *buf_empty cleanup(testBufferDestroy) = testBufferCreate(1);

  case("bufCreate");
  expectEqllu(buf_1->length, 2, "has correct length");
  expectEqllu(buf_1->used, 0, "has correct used");

  case("bufPush");
  panicif(buf_2->used != 0, "unexpected filled buffer");
  bufPush(buf_2, 1);
  expectEqllu(buf_2->used, 1, "increments used");
  bufPush(buf_1, 2);

  case("bufCat");
  bufCat(buf_1, buf_2);
  expectEqllu(buf_1->used, 2, "increases used");
  expectEqli(bufAt(buf_1, 1), 1, "concatenates correctly");
  expectEqllu(buf_2->used, 1, "does not change other buffer");

  bufCat(buf_2, buf_empty);
  expectEqllu(buf_2->used, 1, "pushes an empty list");

  bufCat(buf_empty, buf_2);
  expectEqllu(buf_empty->used, buf_2->used, "pushes to an empty list");

  case("bufRemove");
  bufRemove(buf_empty, bufAt(buf_2, 0), 0);
  expectEqllu(buf_empty->used, 0, "removes element if found");

  bufRemove(buf_empty, bufAt(buf_2, 0), 0);
  expectEqllu(buf_empty->used, 0, "does nothing on empty list");

  size_t initial_len = buf_2->length;
  bufRemove(buf_2, 99, 0);
  expectEqllu(buf_2->used, initial_len, "doesn't change length if not found");

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
