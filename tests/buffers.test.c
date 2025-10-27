#include "../src/buffers.h"
#include "../src/utils.h"
#include "test.h"

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

int main(void) {
  suite(stringCreate);
  suite(stringFrom);
  suite(stringFormat);
  suite(stringClear);
  suite(stringTrim);

  return report();
}
