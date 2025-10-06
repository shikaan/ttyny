#include "../src/buffers.h"
#include "test.h"

void string(void) {
  string_t *subject = strCreate(3);
  bufSet(subject, 0, 'a');
  bufSet(subject, 1, 'b');
  bufSet(subject, 2, 'c');
  expectEqls(subject->data, "abc", 3, "has correct content");
  expectEqli(subject->length, 3, "has correct size");
  strDestroy(&subject);

  subject = strFrom("lol");
  expectEqls(subject->data, "lol", 3, "has correct content");
  expectEqli(subject->length, 3, "has correct size");
  strDestroy(&subject);
}

int main(void) {
  suite(string);
  return report();
}
