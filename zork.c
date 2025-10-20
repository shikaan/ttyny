#include "assets/story.h"
#include "src/ai.h"
#include "src/buffers.h"
#include "src/ui.h"
#include "src/world.h"
#include <ggml.h>
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

void writeUserPrompt(world_t *world, string_t *action, string_t *prompt) {
  location_t *current_location = world->current_location;
  state_t current_location_state = objectGetState(current_location->traits);

  if (action->used) {
    strFmt(prompt, "ACTION: %s\n", action->data);
  }

  strFmtOffset(prompt, prompt->used, "LOCATION: %s (%s) [%s]\n",
               current_location->name, current_location->description,
               current_location->states[current_location_state]);

  strFmtOffset(prompt, prompt->used, "ITEMS:\n");
  for (size_t i = 0; i < current_location->items->used; i++) {
    item_t *object = bufAt(current_location->items, i);
    state_t object_state = objectGetState(object->traits);
    strFmtOffset(prompt, prompt->used, "- %s (%s) [%s]\n", object->name,
                 object->description, object->states[object_state]);
  }

  strFmtOffset(prompt, prompt->used, "EXITS:\n");
  for (size_t i = 0; i < current_location->exits->used; i++) {
    location_t *exit = (location_t *)bufAt(current_location->exits, i);
    state_t object_state = objectGetState(exit->traits);
    strFmtOffset(prompt, prompt->used, "- %s (%s) [%s]\n", exit->name,
                 exit->description, exit->states[object_state]);
  }
}

const char *SYS_PROMPT =
    "You write 2 sentences about LOCATION. "
    "You emphasize EXITS and ITEMS. Explain where EXITS and ITEMS are.\n";

int main(void) {
  string_t *errors = strCreate(512);
  string_t *action = strCreate(512);
  string_t *sys_prompt = strCreate(4096);
  string_t *usr_prompt = strCreate(1024);
  string_t *response = strCreate(4096);

  ai_result_t result;

  ai_t *ai = NULL;
  try(aiCreate("./models/LFM2-1.2B-Q4_k_m.gguf", &LFM2_PROMPT), ai);

  ui_handle_t handle = loadingStart();
  writeUserPrompt(&troll_bridge_world, action, usr_prompt);
  strFmt(sys_prompt, "%s", SYS_PROMPT);

  try(aiSystemPrompt(ai, sys_prompt, usr_prompt, response));
  loadingWait(handle);
  puts(response->data);

  strDestroy(&sys_prompt);

  while (1) {
    printf("> ");
    strReadFrom(action, stdin);

    strClear(response);
    writeUserPrompt(&troll_bridge_world, action, usr_prompt);

    handle = loadingStart();
    try(aiUserPrompt(ai, usr_prompt, response));
    loadingWait(handle);
    puts(response->data);
  }

  strDestroy(&response);
  strDestroy(&usr_prompt);
  strDestroy(&action);
  strDestroy(&errors);
  aiDestory(&ai);
  return 0;
}
