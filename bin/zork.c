#include "../src/ai.h"
#include <stdio.h>
#include <stdlib.h>

void error(const char *string) {
  fprintf(stderr, "error: %s\n", string);
  exit(EXIT_FAILURE);
}

int main(void) {
  string_t *errors = strCreate(2048);

  ai_result_t result = aiInit(MODEL_PATH);
  if (result != AI_RESULT_OK) {
    aiFormatResult(result, errors);
    error(errors->data);
  }

  string_t *prompt = strFrom("What's the capital of Italy?");
  string_t *response = strCreate(4096);

  result = aiGenerate(prompt, response);
  if (result != AI_RESULT_OK) {
    aiFormatResult(result, errors);
    error(errors->data);
  }

  puts(response->data);

  strDestroy(&prompt);
  strDestroy(&response);
  strDestroy(&errors);
  aiTeardown();
  return 0;
}
