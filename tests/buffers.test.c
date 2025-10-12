#include "../src/buffers.h"
#include "test.h"

void stringCreate(void) {
  string_t *subject = strCreate(3);
  expectEqls(subject->data, "", 3, "has correct content");
  expectEqllu(subject->length, 3, "has correct length");
  expectEqllu(subject->used, 0, "has correct used");
  strDestroy(&subject);
}

void stringFrom(void) {
  string_t *subject = strFrom("lol");
  expectEqls(subject->data, "lol", 3, "has correct content");
  expectEqllu(subject->length, 3, "has correct length");
  expectEqllu(subject->used, 3, "has correct used");
  strDestroy(&subject);
}

void stringFormat(void) {
  string_t *subject = strCreate(4);
  strFmt(subject, "%s", "longer");
  expectEqls(subject->data, "long", 4, "truncates content");
  expectEqllu(subject->used, 4, "has correct used");

  strFmt(subject, "%s", "rr");
  expectEqls(subject->data, "rr", subject->length, "overwrites content");
  expectEqllu(subject->used, 2, "has correct used");
  strDestroy(&subject);
}

void stringClear(void) {
  string_t *subject = strFrom("hello");
  size_t length = subject->length;
  strClear(subject);
  expectEqls(subject->data, "", length, "truncates content");
  expectEqllu(subject->length, length, "preserves length");
  expectEqllu(subject->used, 0, "updates used");
  strDestroy(&subject);
}

int main(void) {
  suite(stringCreate);
  suite(stringFrom);
  suite(stringFormat);
  suite(stringClear);

  return report();
}
