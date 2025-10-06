#include "../src/ai.h"
#include <stdio.h>

void error(const char *string) {
  fprintf(stderr, "error: %s\n", string);
  exit(EXIT_FAILURE);
}

int main(void) {
  aiInit(MODEL_PATH);

  string_t *prompt = strFrom("What's the capital of Italy?");
  string_t *response = strCreate(4096);
  aiGenerate(prompt, response);

  puts(response->data);

  return 0;
}
