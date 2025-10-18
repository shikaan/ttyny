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
    "You write 2 sentences about LOCATION."
    "You emphasize EXITS and ITEMS. Explain where EXITS and ITEMS are.\n";

int main(void) {
  string_t *errors = strCreate(512);
  ai_result_t result;

  ai_t *ai = NULL;
  try(aiCreate("./models/LFM2-1.2B-Q4_k_m.gguf", &LFM2_PROMPT), ai);

  string_t *prompt = strCreate(4096);
  strFmt(prompt, "%s", SYS_PROMPT);
  string_t *response = strCreate(4096);

  try(aiPrompt(ai, PROMPT_TYPE_SYS, prompt, response));

  string_t *input = strCreate(256);
  string_t *objects = strCreate(512);
  string_t *exits = strCreate(512);
  string_t *location = strCreate(128);
  string_t *temp = strCreate(512);

  while (1) {
    location_t *current_location = world.current_location;
    state_t current_location_state = getState(current_location->traits);

    strClear(objects);
    for (size_t i = 0; i < current_location->objects->used; i++) {
      object_t *object = bufAt(current_location->objects, i);
      state_t object_state = getState(object->traits);
      strFmt(temp, "\n - %s (%s) [%s]", object->name, object->description,
             object->states[object_state]);
      strCat(objects, temp);
    }
    strClear(temp);

    strClear(exits);
    for (size_t i = 0; i < current_location->exits->used; i++) {
      location_t *exit = (location_t *)bufAt(current_location->exits, i);
      state_t object_state = getState(exit->traits);
      strFmt(temp, "\n - %s (%s) [%s]", exit->name, exit->description,
             exit->states[object_state]);
      strCat(exits, temp);
    }
    strClear(temp);

    strClear(location);
    strFmt(location, "%s (%s) [%s]", current_location->name,
           current_location->description,
           current_location->states[current_location_state]);

    if (input->used) {
      strFmt(prompt,
             "ACTION: %s\n"
             "LOCATION: %s\n"
             "ITEMS: %s\n"
             "EXITS: %s\nOUTPUT:",
             input->data, location->data, objects->data, exits->data);
    } else {
      strFmt(prompt,
             "LOCATION: %s\n"
             "ITEMS: %s\n"
             "EXITS: %s\nOUTPUT:",
             location->data, objects->data, exits->data);
    }

    try(aiPrompt(ai, PROMPT_TYPE_USR, prompt, response));

    puts(response->data);
    strClear(response);
    strClear(prompt);

    printf("> ");
    strReadFrom(input, stdin);
  }

  strDestroy(&prompt);
  strDestroy(&location);
  strDestroy(&response);
  strDestroy(&errors);
  strDestroy(&temp);
  aiDestory(&ai);
  return 0;
}
