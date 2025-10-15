#include "assets/story.h"
#include "ggml.h"
#include "src/ai.h"
#include "src/buffers.h"
#include "src/world.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define try(Prompt, ...)                                                       \
  __VA_OPT__(__VA_ARGS__ =) Prompt;                                            \
  result = aiResultGetLast();                                                  \
  if (result != AI_RESULT_OK) {                                                \
    aiResultFormat(result, errors);                                            \
    fprintf(stderr, "error: %s\n", errors->data);                              \
    exit(EXIT_FAILURE);                                                        \
  }

const char *SYS_PROMPT =
    "You are a creative dungeon master for a text adventure game. Generate "
    "vivid, atmospheric descriptions of fantasy environments and scenarios. "
    "Keep descriptions CONCISE (2-4 sentences) but evocative. "
    "DON'T TAKE ACTIONS ON BEHALF OF THE PLAYER."
    "WHEN ACTION IS UNCLEAR, DESCRIBE CURRENT LOCATION."
    "FOCUS ON OBJECTS. Maintain consistency with the established setting.";

typedef Buffer(string_t *) history_t;
history_t *historyCreate(size_t length) {
  const size_t size = sizeof(history_t);
  history_t *result =
      allocate(size + ((unsigned)(length) * sizeof(string_t *)));
  if (!result) {
    return NULL;
  }
  result->length = length;
  result->used = 0;
  return result;
}

void historyPush(history_t *self, const char *string) {
  // if buffer is full, make space for one mosre string
  if (self->used >= self->length) {
    string_t *first = bufAt(self, 0);
    strDestroy(&first);
    for (size_t i = 1; i < self->length - 1; i++) {
      bufSet(self, i - 1, bufAt(self, i));
    }
    self->used--;
  }

  self->data[self->used] = strFrom(string);
  self->used++;
}

int main(void) {
  string_t *errors = strCreate(512);
  ai_result_t result;

  ai_t *ai = NULL;
  try(aiCreate("./models/LFM2-1.2B-Q4_k_m.gguf", &LFM2_PROMPT), ai);

  string_t *prompt = strCreate(4096);
  strFmt(prompt, "%s", SYS_PROMPT);
  string_t *response = strCreate(4096);

  try(aiPrompt(ai, PROMPT_TYPE_SYS, prompt, response));

  char input_buffer[256] = {};
  string_t *objects = strCreate(512);
  string_t *actions = strCreate(512);
  history_t *history = historyCreate(10);

  strFmt(prompt, "%s", "Look Around");
  while (1) {
    location_t *current_location = world.current_location;
    state_t current_location_state = getState(current_location->traits);

    strClear(objects);
    for (size_t i = 0; i < current_location->objects->used; i++) {
      object_t *object = bufAt(current_location->objects, i);
      state_t object_state = getState(object->traits);
      strFmt(objects, "%s\n  - %s: %s", objects->data, object->name,
             object->descriptions[object_state]);
    }

    strClear(actions);
    for (size_t i = 0; i < history->used; i++) {
      string_t *entry = bufAt(history, i);
      strFmt(objects, "%s\n  - %s", actions->data, entry->data);
    }

    strFmt(prompt,
           "World: %s\n"
           "Location: %s - %s\n"
           "Objects: %s\n"
           "Last User Actions: %s\n"
           "Player Action: %s.\n"
           "Respond to player actions describing the location. DON'T OFFER "
           "OPTIONS. USE SECOND PERSON.",
           world.context, world.current_location->name,
           world.current_location->descriptions[current_location_state],
           objects->data, actions->data, input_buffer);
    try(aiPrompt(ai, PROMPT_TYPE_USR, prompt, response));

    puts(response->data);
    strClear(response);
    strClear(prompt);

    printf("> ");
    fgets(input_buffer, 256, stdin);
    historyPush(history, input_buffer);
  }

  strDestroy(&prompt);
  strDestroy(&response);
  strDestroy(&errors);
  aiDestory(&ai);
  return 0;
}
