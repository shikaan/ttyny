#include "src/ai.h"
#include <stdio.h>
#include <stdlib.h>

void error(const char *string) {
  fprintf(stderr, "error: %s\n", string);
  exit(EXIT_FAILURE);
}

int main(void) {
  string_t *errors = strCreate(512);
  ai_result_t result;

  ai_t *ai = aiCreate("./models/LFM2-1.2B-Q4_k_m.gguf", &LFM2_PROMPT);
  result = aiResultGetLast();

  if (result != AI_RESULT_OK) {
    aiResultFormat(result, errors);
    error(errors->data);
  }

  string_t *prompt = strFrom(
      "You are the NARRATOR of a classic interactive text adventure."
      "Output only evocative narration (present tense, second person)."
      "Do NOT invent new objects or characters except those explicitly listed."
      "Do NOT show response options."
      "NEVER show roles."
      "Keep under 110 words. Never show internal reasoning."
      "No player commands in output.");
  string_t *response = strCreate(4096);

  aiPrompt(ai, PROMPT_TYPE_SYS, prompt, response);
  if (result != AI_RESULT_OK) {
    aiResultFormat(result, errors);
    error(errors->data);
  }

  char user_input[256] = {};
  while (1) {
    aiPrompt(ai, PROMPT_TYPE_USR, prompt, response);
    result = aiResultGetLast();
    if (result != AI_RESULT_OK) {
      aiResultFormat(result, errors);
      error(errors->data);
    }

    puts(response->data);
    strClear(response);

    printf("> ");
    fgets(user_input, 256, stdin);
    strFmt(prompt, user_input, 0);
  }

  strDestroy(&prompt);
  strDestroy(&response);
  strDestroy(&errors);
  aiDestory(&ai);
  return 0;
}
