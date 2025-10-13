#include "src/ai.h"
#include "src/buffers.h"
#include <stdio.h>
#include <stdlib.h>

void error(const char *string) {
  fprintf(stderr, "error: %s\n", string);
  exit(EXIT_FAILURE);
}

const char *SYS_PROMPT =
    "You are a creative dungeon master for a text adventure game. Generate "
    "vivid, atmospheric descriptions of fantasy environments and scenarios. "
    "Keep descriptions concise (2-4 sentences) but evocative. Focus on sensory "
    "details and actionable elements the player can interact with. Maintain "
    "consistency with the established setting.";

int main(void) {
  string_t *errors = strCreate(512);
  ai_result_t result;

  ai_t *ai = aiCreate("./models/LFM2-1.2B-Q4_k_m.gguf", &LFM2_PROMPT);
  result = aiResultGetLast();

  if (result != AI_RESULT_OK) {
    aiResultFormat(result, errors);
    error(errors->data);
  }

  string_t *prompt = strFrom(SYS_PROMPT);
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
