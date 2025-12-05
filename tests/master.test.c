#include "test.h"

#include "../src/lib/buffers.h"
#include "../src/master.h"
#include "../src/utils.h"
#include <stddef.h>

typedef struct {
  const char *input;
  int result;
} validate_case_t;

void validate(void) {
  string_t *buffer cleanup(strDestroy) = strCreate(1024);
  words_t *required cleanup(wordsDestroy) = wordsCreate(1);

  case("forbidden words");
  validate_case_t forbidden_words[] = {
      {"EXIT", false},   {"LOCATION", false}, {"EXITS", false},
      {"ITEMS", false},  {"ACTION", false},   {"DESCRIPTION", false},
      {"STATE", false},  {"TARGET", false},   {"inventory", false},
      {"player", false}, {"player's", false}, {"location", false},
  };

  for (size_t i = 0; i < arrLen(forbidden_words); i++) {
    validate_case_t test_case = forbidden_words[i];
    strFmt(buffer, "%s", test_case.input);
    expect(masterIsValidResponse(buffer, required) == test_case.result,
           test_case.input, "Unexpected validation result");
  }

  case("required words");
  bufPush(required, "you");
  strFmt(buffer, "%s", "you");
  expectTrue(masterIsValidResponse(buffer, required), "exact match");
  bufClear(required, NULL);

  bufPush(required, "you");
  strFmt(buffer, "%s", "you are yellow");
  expectTrue(masterIsValidResponse(buffer, required), "single: output contains matching");
  bufClear(required, NULL);

  bufPush(required, "you");
  strFmt(buffer, "%s", "yo are yellow");
  expectFalse(masterIsValidResponse(buffer, required), "single: output contains subset");
  bufClear(required, NULL);

  bufPush(required, "yo");
  strFmt(buffer, "%s", "you are yellow");
  expectFalse(masterIsValidResponse(buffer, required), "single: output contains superset");
  bufClear(required, NULL);

  bufPush(required, "shiny coin");
  strFmt(buffer, "%s", "yellow is shiny coin");
  expectTrue(masterIsValidResponse(buffer, required), "multiple: output contains matching");
  bufClear(required, NULL);

  bufPush(required, "shiny coin");
  strFmt(buffer, "%s", "rare is shiny something else");
  expectFalse(masterIsValidResponse(buffer, required), "multiple: output contains subset");
  bufClear(required, NULL);

  bufPush(required, "shiny coin");
  strFmt(buffer, "%s", "rare is shiny coincidence");
  expectFalse(masterIsValidResponse(buffer, required), "multiple: output contains superset");
  bufClear(required, NULL);
}

int main(void) {
  suite(validate);
  return report();
}
