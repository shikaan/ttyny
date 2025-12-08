#include "master.h"
#include "ai.h"
#include "lib/alloc.h"
#include "lib/buffers.h"
#include "lib/map.h"
#include "utils.h"
#include "world/item.h"
#include "world/object.h"
#include "world/world.h"

#include "configs/qwen.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

// Define namespace constants
const char *LOCATION_NAMESPACE = "location";
const char *ITEM_NAMESPACE = "item";
const char *OBJECT_NAMESPACE = "object";

words_t *wordsCreate(size_t len) {
  words_t *result;
  bufCreate(words_t, const char *, result, len);
  return result;
}

void wordsDestroy(words_t **self) { deallocate(self); }

static words_t STOP_WORDS =
    bufConst(4, "inventory", "player", "player's", "location");
static words_t STOP_WORDS_CASE = bufConst(7, "EXITS", "EXIT", "ITEMS", "ACTION",
                                          "DESCRIPTION", "STATE", "TARGET");
static words_t ACTION_MUST_HAVES = bufConst(1, "you");

static void summarizeLocation(const location_t *location, string_t *summary) {
  strFmt(summary,
         "LOCATION: %s\n"
         "DESCRIPTION: %s\n",
         location->object.name,
         bufAt(location->object.descriptions, location->object.state));

  size_t i = 0;
  if (!bufIsEmpty(location->items)) {
    strFmtAppend(summary, "ITEMS: ");
    bufEach(location->items, i) {
      item_t *item = bufAt(location->items, i);
      if (i > 0)
        strFmtAppend(summary, ", ");
      strFmtAppend(summary, "%s", item->object.name);
    }
  }

  strFmtAppend(summary, "\nEXITS: ");
  bufEach(location->exits, i) {
    location_t *exit = (location_t *)bufAt(location->exits, i);
    if (i > 0)
      strFmtAppend(summary, ", ");
    strFmtAppend(summary, "%s", exit->object.name);
  }
  strFmtAppend(summary, "\n");
}

master_t *masterCreate(world_t *world) {
  panicif(!world || !world->items || !world->locations,
          "need to initialize world first");
  master_t *master = allocate(sizeof(master_t));
  if (!master) {
    error("cannot allocate master");
    return NULL;
  }

  ai_result_t result;
  master->ai = aiCreate(&NARRATOR_CONFIG, &result);
  if (result != AI_RESULT_OK) {
    error("cannot allocate AI for master");
    return NULL;
  }

  master->prompt = strCreate(4096);
  if (!master->prompt) {
    error("cannot allocate prompt buffer");
    masterDestroy(&master);
    return NULL;
  }

  master->summary = strCreate(4096);
  if (!master->summary) {
    error("cannot allocate summary buffer");
    masterDestroy(&master);
    return NULL;
  }

  master->descriptions = mapCreate(world->items->len + world->locations->len);
  if (!master->descriptions) {
    error("cannot allocate summary buffer");
    masterDestroy(&master);
    return NULL;
  }

  return master;
}

static char cache_key_buffer[256] = {};
static map_key_t makeCacheKey(object_name_t name, const char *namespace) {
  snprintf(cache_key_buffer, 256, "%s.%s", namespace, name);
  return cache_key_buffer;
}

static const char *WORD_BREAK = " \t\r\n:-*'.,";

static int hasStopWords(string_t *response) {
  panicif(!response, "missing response");

  string_t *input cleanup(strDestroy) = strDup(response);
  if (!input)
    return 0;

  char *saveptr = NULL;
  char *token = strtok_r(input->data, WORD_BREAK, &saveptr);
  while (token) {
    for (size_t i = 0; i < STOP_WORDS.len; i++) {
      const char *word = bufAt(&STOP_WORDS, i);
      if (strcasecmp(token, word) == 0) {
        info("Invalid: Found a STOP WORD %s", word);
        return 1;
      }
    }

    for (size_t i = 0; i < STOP_WORDS_CASE.len; i++) {
      const char *word = bufAt(&STOP_WORDS_CASE, i);
      if (strcmp(token, word) == 0) {
        info("Invalid: Found a STOP WORD %s", word);
        return 1;
      }
    }

    token = strtok_r(NULL, WORD_BREAK, &saveptr);
  }

  return 0;
}

static int hasAllMustHaves(string_t *response, words_t *must_haves) {
  panicif(!response, "missing response");
  if (!must_haves)
    return 1;

  size_t i = 0;
  bufEach(must_haves, i) {
    const char *word = bufAt(must_haves, i);
    char *word_position = strcasestr(response->data, word);
    if (!word_position) {
      info("Invalid: Missing must have %s", word);
      return 0;
    }

    // if the next char at the end of the string is not a break return false
    size_t len = strlen(word);
    if (!strchr(WORD_BREAK, word_position[len])) {
      return 0;
    }
  }

  return 1;
}

int masterIsValidResponse(string_t *response, words_t *must_haves) {
  return !hasStopWords(response) && hasAllMustHaves(response, must_haves);
}

static void generateAndValidate(ai_t *ai, const string_t *prompt,
                                string_t *response, words_t *must_haves) {
  debug("Prompt:\n%s", prompt->data);
  int valid = 0;
  ai_result_t result;
#ifndef DEBUG
  static const size_t MAX_ATTEMPTS = 20;
#else
  static const size_t MAX_ATTEMPTS = 1;
#endif
  for (size_t i = 0; i < MAX_ATTEMPTS && !valid; i++) {
    strClear(response);
    result = aiReset(ai);
    panicif(result != AI_RESULT_OK, "cannot reset model state");
    result = aiGenerate(ai, prompt, response);
    if (result != AI_RESULT_OK) {
      valid = 0;
      continue;
    }
    valid = masterIsValidResponse(response, must_haves);

    if (!valid)
      debug("Rejected:\n%s\n", response->data);
  }
  if (!valid) {
    error("Invalid output: giving up.")
  }
}

void masterDescribeLocation(master_t *self, const location_t *location,
                            string_t *description) {
  map_key_t cache_key = makeCacheKey(location->object.name, LOCATION_NAMESPACE);
  char *cached = mapGet(self->descriptions, cache_key);
  if (cached) {
    debug("returning from cache: %s\n", cache_key);
    strFmt(description, "%s", cached);
    return;
  }
  debug("cache miss: %s\n", cache_key);

  const config_t *config = self->ai->configuration;
  const string_t *usr_prompt_tpl = config->prompt_templates[PROMPT_TYPE_USR];
  const string_t *sys_prompt_tpl = config->prompt_templates[PROMPT_TYPE_SYS];
  const string_t *res_prompt_tpl = config->prompt_templates[PROMPT_TYPE_RES];

  strFmt(self->prompt, sys_prompt_tpl->data, MASTER_WORLD_DESC_SYS_PROMPT.data);

  summarizeLocation(location, self->summary);
  strFmtAppend(self->prompt, usr_prompt_tpl->data, self->summary->data);
  strFmtAppend(self->prompt, res_prompt_tpl->data, "");

  // TODO: this seems inefficient: this list can never be longer than all
  // elements + all exits, it could be statically allocated
  words_t *must_haves cleanup(wordsDestroy) =
      wordsCreate(location->items->len + location->exits->len);

  size_t i = 0;
  bufEach(location->items, i) {
    item_t *item = bufAt(location->items, i);
    bufPush(must_haves, item->object.name);
  }

  bufEach(location->exits, i) {
    location_t *exit = (location_t *)bufAt(location->exits, i);
    bufPush(must_haves, exit->object.name);
  }

  generateAndValidate(self->ai, self->prompt, description, must_haves);

  char *description_data = strdup(description->data);
  char *description_key = strdup(cache_key);
  (void)mapSet(self->descriptions, description_key, description_data);
  debug("written cache at: %s\n", cache_key);
}

void masterReadItem(master_t *self, const item_t *item, string_t *description) {
  const object_t object = item->object;
  map_key_t cache_key = makeCacheKey(object.name, ITEM_NAMESPACE);
  debug("reading cache key: %s\n", cache_key);
  const char *state_desc = bufAt(object.descriptions, object.state);
  strFmt(description, "%s", state_desc);
  char *copy = strdup(description->data);
  (void)mapSet(self->descriptions, cache_key, copy);
  debug("written cache at: %s\n", cache_key);
}

void masterDescribeObject(master_t *self, const object_t *object,
                          string_t *description) {
  map_key_t cache_key = makeCacheKey(object->name, OBJECT_NAMESPACE);

  char *cached = mapGet(self->descriptions, cache_key);
  if (cached) {
    debug("returning from cache: %s\n", cache_key);
    strFmt(description, "%s", cached);
    return;
  }
  debug("cache miss: %s\n", cache_key);

  const config_t *config = self->ai->configuration;
  const string_t *sys_prompt_tpl = config->prompt_templates[PROMPT_TYPE_SYS];
  const string_t *res_prompt_tpl = config->prompt_templates[PROMPT_TYPE_RES];

  strFmt(self->prompt, sys_prompt_tpl->data,
         MASTER_OBJECT_DESC_SYS_PROMPT.data);

  strFmtAppend(self->prompt, "\nITEM:\n name: %s\n description: %s\n",
               object->name, bufAt(object->descriptions, object->state));

  strFmtAppend(self->prompt, res_prompt_tpl->data, "");

  generateAndValidate(self->ai, self->prompt, description, NULL);

  char *copy = strdup(description->data);
  (void)mapSet(self->descriptions, cache_key, copy);
  debug("written cache at: %s\n", cache_key);
}

void masterDescribeAction(master_t *self, const world_t *world,
                          const string_t *input, const object_t *object,
                          const object_t *transition_target,
                          object_state_t transition_target_initial_state,
                          string_t *comment) {
  const config_t *config = self->ai->configuration;
  const string_t *sys_prompt_tpl = config->prompt_templates[PROMPT_TYPE_SYS];
  const string_t *usr_prompt_tpl = config->prompt_templates[PROMPT_TYPE_USR];
  const string_t *res_prompt_tpl = config->prompt_templates[PROMPT_TYPE_RES];

  // Need to do it first, else it scrambles the self->prompt
  masterDescribeLocation(self, world->location, self->summary);

  strFmt(self->prompt, sys_prompt_tpl->data, MASTER_ACTION_SYS_PROMPT.data);
  strFmtAppend(self->prompt, usr_prompt_tpl->data, "look around");
  strFmtAppend(self->prompt, res_prompt_tpl->data, self->summary->data);

  if (transition_target && transition_target != object) {
    char *target_initial_desc =
        bufAt(transition_target->descriptions, transition_target_initial_state);
    char *target_desc =
        bufAt(transition_target->descriptions, transition_target->state);
    strFmt(self->summary,
           "ACTION: %s\n"
           "TARGET: %s (%s)\n",
           input->data, object->name,
           bufAt(object->descriptions, object->state));

    if (transition_target->state != transition_target_initial_state) {
      strFmtAppend(self->summary,
                   "TRANSITION_TARGET: %s\n"
                   "TRANSITION_INITIAL_STATE: %s\n"
                   "TRANSITION_FINAL_STATE: %s\n",
                   transition_target->name, target_initial_desc, target_desc);
    }
  } else {
    strFmt(self->summary,
           "ACTION: %s\n"
           "TARGET: %s (%s)\n",
           input->data, object->name,
           bufAt(object->descriptions, object->state));
  }

  strFmtAppend(self->prompt, usr_prompt_tpl->data, self->summary->data);
  strFmtAppend(self->prompt, res_prompt_tpl->data, "");

  generateAndValidate(self->ai, self->prompt, comment, &ACTION_MUST_HAVES);
}

void masterDescribeEndGame(master_t *self, const string_t *last_action,
                           const world_t *world, game_state_t state,
                           string_t *description) {
  if (state == GAME_STATE_CONTINUE) {
    strClear(description);
    return;
  }

  const config_t *config = self->ai->configuration;
  const string_t *sys_prompt_tpl = config->prompt_templates[PROMPT_TYPE_SYS];
  const string_t *usr_prompt_tpl = config->prompt_templates[PROMPT_TYPE_USR];
  const string_t *res_prompt_tpl = config->prompt_templates[PROMPT_TYPE_RES];

  // Need to do it before everything, else it scrambles the prompt
  masterDescribeLocation(self, world->location, self->summary);

  strFmt(self->prompt, sys_prompt_tpl->data, MASTER_END_GAME_SYS_PROMPT.data);

  // Using a shot to provide some context
  strFmtAppend(self->prompt, usr_prompt_tpl->data, "look around");
  strFmtAppend(self->prompt, res_prompt_tpl->data, self->summary->data);

  strFmt(self->summary, "ACTION: %s\nENDING: %s\nREASON: %s", last_action->data,
         state == GAME_STATE_VICTORY ? "victory" : "death", world->end_game);
  strFmtAppend(self->prompt, usr_prompt_tpl->data, self->summary->data);
  strFmtAppend(self->prompt, res_prompt_tpl->data, "");

  generateAndValidate(self->ai, self->prompt, description, &ACTION_MUST_HAVES);
}

void masterForget(master_t *self, const object_t *object,
                  const char *namespace) {
  map_key_t cache_key = makeCacheKey(object->name, namespace);
  char *value = mapDelete(self->descriptions, cache_key);
  deallocate(&value);
}

void masterDestroy(master_t **self) {
  if (!self || !*self)
    return;

  aiDestroy(&(*self)->ai);
  strDestroy(&(*self)->prompt);
  strDestroy(&(*self)->summary);

  for (map_size_t i = 0; i < (*self)->descriptions->size; i++) {
    char *memory = (*self)->descriptions->values[i];
    deallocate(&memory);
  }
  mapDestroy(&(*self)->descriptions);

  deallocate(self);
}
