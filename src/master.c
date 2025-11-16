#include "master.h"
#include "ai.h"
#include "alloc.h"
#include "buffers.h"
#include "map.h"
#include "utils.h"
#include "world.h"

#include "configs/qwen.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef Buffer(const char *) words_t;

static inline words_t *wordsCreate(size_t len) {
  words_t *result;
  makeBufCreate(words_t, const char *, result, len);
  return result;
}

static inline void wordsDestroy(words_t **self) { deallocate(self); }

static words_t STOP_WORDS =
    bufConst(4, "inventory", "player", "player's", "location");
static words_t STOP_WORDS_CASE = bufConst(3, "EXITS", "EXIT", "ITEMS");
static words_t ACTION_MUST_HAVES = bufConst(1, "you");

static void describeLocation(const location_t *location, string_t *summary) {
  strFmt(summary,
         "LOCATION: %s\n"
         "DESCRIPTION: %s\n"
         "STATE: %s\n",
         location->object.name, location->object.description,
         bufAt(location->object.state_descriptions,
               location->object.current_state));

  if (location->items->used) {
    strFmtAppend(summary, "\nITEMS: ");
    for (size_t i = 0; i < location->items->used; i++) {
      item_t *item = bufAt(location->items, i);
      if (i > 0)
        strFmtAppend(summary, ", ");
      strFmtAppend(summary, "%s", item->object.name);
    }
  }

  strFmtAppend(summary, "\nEXITS: ");
  for (size_t i = 0; i < location->exits->used; i++) {
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

  master->descriptions = mapCreate(world->items->used + world->locations->used);
  if (!master->descriptions) {
    error("cannot allocate summary buffer");
    masterDestroy(&master);
    return NULL;
  }

  return master;
}

static int hasStopWords(string_t *response) {
  panicif(!response, "missing response");
  static const char *WORD_BREAK = " \t\r\n:-*'";

  string_t *input cleanup(strDestroy) = strDup(response);
  if (!input)
    return 0;

  char *saveptr = NULL;
  char *token = strtok_r(input->data, WORD_BREAK, &saveptr);
  while (token) {
    for (size_t i = 0; i < STOP_WORDS.used; i++) {
      const char *word = bufAt(&STOP_WORDS, i);
      if (strcasecmp(token, word) == 0) {
        info("Invalid: Found a STOP WORD %s", word);
        return 1;
      }
    }

    for (size_t i = 0; i < STOP_WORDS_CASE.used; i++) {
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

  for (size_t i = 0; i < must_haves->used; i++) {
    const char *word = bufAt(must_haves, i);
    if (!strcasestr(response->data, word)) {
      info("Invalid: Missing must have %s", word);
      return 0;
    }
  }

  return 1;
}

static void generateAndValidate(ai_t *ai, const string_t *prompt,
                                string_t *response, words_t *must_haves) {
  int valid = 0;
  ai_result_t result;
  for (size_t i = 0; i < 20 && !valid; i++) {
    strClear(response);
    aiReset(ai, &result);
    panicif(result != AI_RESULT_OK, "cannot reset model state");
    aiGenerate(ai, &result, prompt, response);
    if (result != AI_RESULT_OK) {
      valid = 0;
      continue;
    }
    valid = !hasStopWords(response) && hasAllMustHaves(response, must_haves);

    if (!valid)
      debug("Rejected:\n%s\n", response->data);
  }
  if (!valid) {
    info("Invalid: giving up")
  }
}

void masterDescribeLocation(master_t *self, const location_t *location,
                            string_t *description) {
  key_t cache_key = location->object.name;
  char *cached = mapGet(self->descriptions, cache_key);
  if (cached) {
    strFmt(description, "%s", cached);
    return;
  }

  const config_t *config = self->ai->configuration;
  const string_t *usr_prompt_tpl = config->prompt_templates[PROMPT_TYPE_USR];
  const string_t *sys_prompt_tpl = config->prompt_templates[PROMPT_TYPE_SYS];
  const string_t *res_prompt_tpl = config->prompt_templates[PROMPT_TYPE_RES];

  strFmt(self->prompt, sys_prompt_tpl->data, MASTER_WORLD_DESC_SYS_PROMPT.data);

  describeLocation(location, self->summary);
  strFmtAppend(self->prompt, usr_prompt_tpl->data, self->summary->data);
  strFmtAppend(self->prompt, res_prompt_tpl->data, "");

  // TODO: this seems inefficient: this list can never be longer than all
  // elements + all exits, it could be statically allocated
  words_t *must_haves cleanup(wordsDestroy) =
      wordsCreate(location->items->used + location->exits->used);

  for (size_t i = 0; i < location->items->used; i++) {
    item_t *item = bufAt(location->items, i);
    bufPush(must_haves, item->object.name);
  }

  for (size_t i = 0; i < location->exits->used; i++) {
    location_t *exit = (location_t *)bufAt(location->exits, i);
    bufPush(must_haves, exit->object.name);
  }

  debug("Prompt:\n%s\n", self->prompt->data);
  generateAndValidate(self->ai, self->prompt, description, must_haves);

  char *copy = strdup(description->data);
  (void)mapSet(self->descriptions, cache_key, copy);
}

void masterDescribeObject(master_t *self, const object_t *object,
                          string_t *description) {
  key_t cache_key = object->name;
  char *cached = mapGet(self->descriptions, cache_key);
  if (cached) {
    strFmt(description, "%s", cached);
    return;
  }
  const config_t *config = self->ai->configuration;
  const string_t *sys_prompt_tpl = config->prompt_templates[PROMPT_TYPE_SYS];
  const string_t *res_prompt_tpl = config->prompt_templates[PROMPT_TYPE_RES];

  strFmt(self->prompt, sys_prompt_tpl->data,
         MASTER_OBJECT_DESC_SYS_PROMPT.data);

  strFmtAppend(self->prompt,
               "\nITEM:\n name: %s\n description: %s\n state: %s\n",
               object->name, object->description,
               bufAt(object->state_descriptions, object->current_state));

  strFmtAppend(self->prompt, res_prompt_tpl->data, "");

  generateAndValidate(self->ai, self->prompt, description, NULL);

  char *copy = strdup(description->data);
  (void)mapSet(self->descriptions, cache_key, copy);
}

void masterDescribeSuccess(master_t *self, const world_t *world,
                           const string_t *input, string_t *comment) {
  const config_t *config = self->ai->configuration;
  const string_t *sys_prompt_tpl = config->prompt_templates[PROMPT_TYPE_SYS];
  const string_t *usr_prompt_tpl = config->prompt_templates[PROMPT_TYPE_USR];
  const string_t *res_prompt_tpl = config->prompt_templates[PROMPT_TYPE_RES];

  strFmt(self->prompt, sys_prompt_tpl->data, MASTER_SUCCESS_SYS_PROMPT.data);
  describeLocation(world->current_location, self->summary);
  strFmtAppend(self->prompt, "\n%s", self->summary->data);
  strFmtAppend(self->prompt, usr_prompt_tpl->data, input->data);
  strFmtAppend(self->prompt, res_prompt_tpl->data, "");

  generateAndValidate(self->ai, self->prompt, comment, &ACTION_MUST_HAVES);
}

void masterDescribeEndGame(master_t *self, const world_t *world,
                           game_state_t state, string_t *description) {

  if (state == GAME_STATE_CONTINUE) {
    strClear(description);
    return;
  }

  const config_t *config = self->ai->configuration;
  const string_t *sys_prompt_tpl = config->prompt_templates[PROMPT_TYPE_SYS];
  const string_t *usr_prompt_tpl = config->prompt_templates[PROMPT_TYPE_USR];
  const string_t *res_prompt_tpl = config->prompt_templates[PROMPT_TYPE_RES];

  strFmt(self->prompt, sys_prompt_tpl->data, MASTER_END_GAME_SYS_PROMPT.data);
  strFmtAppend(self->prompt, usr_prompt_tpl->data, world->end_game[state]);
  strFmtAppend(self->prompt, res_prompt_tpl->data, "");

  generateAndValidate(self->ai, self->prompt, description, &ACTION_MUST_HAVES);
}

void masterForget(master_t *self, const object_t *object) {
  key_t cache_key = object->name;
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
  // TODO should we clean also this just in case?
  mapDestroy(&(*self)->descriptions);

  deallocate(self);
}
