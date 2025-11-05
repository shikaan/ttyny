#include "dm.h"
#include "ai.h"
#include "alloc.h"
#include "buffers.h"
#include "map.h"
#include "ring.h"
#include "utils.h"
#include "world.h"

#include "configs/qwen.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  const char *action;
  const failure_type_t failure;
  const char *response;
} failure_shot_t;

typedef Buffer(const char *) words_t;

static inline words_t *wordsCreate(size_t len) {
  words_t *result;
  makeBufCreate(words_t, const char *, result, len);
  return result;
}

static inline void wordsDestroy(words_t **self) { deallocate(self); }

static words_t STOP_WORDS =
    bufConst(6, "item", "items", "inventory", "player", "player's", "location");
static words_t STOP_WORDS_CASE = bufConst(3, "EXITS", "EXIT", "ITEMS");
static words_t STOP_CHARS = bufConst(3, "[", "(", "*");
static words_t ACTION_MUST_HAVES = bufConst(1, "you");

static void summarize(const world_t *world, string_t *summary) {
  location_t *current_location = world->current_location;

  strFmt(summary, "LOCATION: %s (%s) [%s]\n", current_location->object.name,
         current_location->object.description,
         bufAt(current_location->object.state_descriptions,
               current_location->object.current_state));

  if (current_location->items->used) {
    strFmtAppend(summary, "ITEMS:\n");
    for (size_t i = 0; i < current_location->items->used; i++) {
      item_t *item = bufAt(current_location->items, i);
      strFmtAppend(
          summary, "  - %s (%s) [%s]\n", item->object.name,
          item->object.description,
          bufAt(item->object.state_descriptions, item->object.current_state));
    }
  }

  strFmtOffset(summary, summary->used, "EXITS:\n");
  for (size_t i = 0; i < current_location->exits->used; i++) {
    location_t *exit = (location_t *)bufAt(current_location->exits, i);
    strFmtAppend(
        summary, "  - %s (%s) [%s]\n", exit->object.name,
        exit->object.description,
        bufAt(exit->object.state_descriptions, exit->object.current_state));
  }
}

dm_t *dmCreate(world_t *world) {
  dm_t *dm = allocate(sizeof(dm_t));
  panicif(!dm, "cannot allocate narrator");

  ai_result_t result;
  dm->ai = aiCreate(&NARRATOR_CONFIG, &result);
  panicif(!dm->ai, "cannot allocate AI for narrator");

  dm->prompt = strCreate(4096);
  panicif(!dm->prompt, "cannot allocate prompt buffer");

  dm->summary = strCreate(4096);
  panicif(!dm->summary, "cannot allocate summary buffer");

  dm->descriptions = mapCreate(world->items->used + world->locations->used);
  panicif(!dm->descriptions, "cannot allocate memory");

  dm->history = ringCreate(3);
  panicif(!dm->history, "cannot allocate history");

  return dm;
}

static int hasStopWords(string_t *response) {
  static const char *WORD_BREAK = " \t\r\n:-*'";
  panicif(!response, "missing response");

  string_t *input cleanup(strDestroy) = strDup(response);
  if (!input)
    return 0;

  char *saveptr = NULL;
  char *token = strtok_r(input->data, WORD_BREAK, &saveptr);
  while (token) {
    for (size_t i = 0; i < STOP_WORDS.used; i++) {
      const char *word = bufAt(&STOP_WORDS, i);
      if (strcasecmp(token, word) == 0) {
        return 1;
      }
    }

    for (size_t i = 0; i < STOP_WORDS_CASE.used; i++) {
      const char *word = bufAt(&STOP_WORDS_CASE, i);
      if (strcmp(token, word) == 0) {
        return 1;
      }
    }

    for (size_t i = 0; i < STOP_CHARS.used; i++) {
      const char *word = bufAt(&STOP_CHARS, i);
      if (strchr(token, word[0]) != NULL) {
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
    panicif(result != AI_RESULT_OK, "cannot generate response");
    valid = !hasStopWords(response) && hasAllMustHaves(response, must_haves);
  }
}

void dmDescribeWorld(dm_t *self, const world_t *world, string_t *description) {
  key_t cache_key = world->current_location->object.name;
  char *cached = mapGet(self->descriptions, cache_key);
  if (cached) {
    strFmt(description, "%s", cached);
    char *evicted = ringPushUniq(self->history, cached);
    deallocate(&evicted);
    return;
  }

  const config_t *config = self->ai->configuration;
  const string_t *sys_prompt_tpl = config->prompt_templates[PROMPT_TYPE_SYS];
  const string_t *res_prompt_tpl = config->prompt_templates[PROMPT_TYPE_RES];

  strFmt(self->prompt, sys_prompt_tpl->data,
         NARRATOR_WORLD_DESC_SYS_PROMPT.data);

  summarize(world, self->summary);
  strFmtAppend(self->prompt, "\n%s", self->summary->data);
  strFmtAppend(self->prompt, res_prompt_tpl->data, "");

  // TODO: this seems inefficient: this list can never be longer than all
  // elements + all exits, it could be statically allocated
  words_t *must_haves cleanup(wordsDestroy) =
      wordsCreate(world->current_location->items->used +
                  world->current_location->exits->used);

  for (size_t i = 0; i < world->current_location->items->used; i++) {
    item_t *item = bufAt(world->current_location->items, i);
    bufPush(must_haves, item->object.name);
  }

  for (size_t i = 0; i < world->current_location->exits->used; i++) {
    location_t *exit = (location_t *)bufAt(world->current_location->exits, i);
    bufPush(must_haves, exit->object.name);
  }

  generateAndValidate(self->ai, self->prompt, description, must_haves);

  char *copy = strdup(description->data);
  (void)mapSet(self->descriptions, cache_key, copy);
  string_t *evicted = ringPushUniq(self->history, copy);
  deallocate(&evicted);
}

void dmDescribeObject(dm_t *self, const object_t *object,
                      string_t *description) {
  key_t cache_key = object->name;
  char *cached = mapGet(self->descriptions, cache_key);
  if (cached) {
    strFmt(description, "%s", cached);
    char *evicted = ringPushUniq(self->history, cached);
    deallocate(&evicted);
    return;
  }
  const config_t *config = self->ai->configuration;
  const string_t *sys_prompt_tpl = config->prompt_templates[PROMPT_TYPE_SYS];
  const string_t *res_prompt_tpl = config->prompt_templates[PROMPT_TYPE_RES];

  strFmt(self->prompt, sys_prompt_tpl->data,
         NARRATOR_OBJECT_DESC_SYS_PROMPT.data);

  strFmtAppend(self->prompt,
               "\nITEM:\n name: %s\n description: %s\n state: %s\n",
               object->name, object->description,
               bufAt(object->state_descriptions, object->current_state));

  strFmtAppend(self->prompt, res_prompt_tpl->data, "");

  generateAndValidate(self->ai, self->prompt, description, NULL);

  char *copy = strdup(description->data);
  (void)mapSet(self->descriptions, cache_key, copy);
  string_t *evicted = ringPushUniq(self->history, copy);
  deallocate(&evicted);
}

void dmDescribeSuccess(dm_t *self, const world_t *world, const string_t *input,
                       string_t *comment) {
  const config_t *config = self->ai->configuration;
  const string_t *sys_prompt_tpl = config->prompt_templates[PROMPT_TYPE_SYS];
  const string_t *usr_prompt_tpl = config->prompt_templates[PROMPT_TYPE_USR];
  const string_t *res_prompt_tpl = config->prompt_templates[PROMPT_TYPE_RES];

  strFmt(self->prompt, sys_prompt_tpl->data, NARRATOR_SUCCESS_SYS_PROMPT.data);
  summarize(world, self->summary);
  strFmtAppend(self->prompt, "\n%s", self->summary->data);
  strFmtAppend(self->prompt, usr_prompt_tpl->data, input->data);
  strFmtAppend(self->prompt, res_prompt_tpl->data, "");

  generateAndValidate(self->ai, self->prompt, comment, &ACTION_MUST_HAVES);
}

void dmDescribeEndGame(dm_t *self, const world_t *world, game_state_t state,
                       string_t *description) {

  if (state == GAME_STATE_CONTINUE) {
    strClear(description);
    return;
  }

  const config_t *config = self->ai->configuration;
  const string_t *sys_prompt_tpl = config->prompt_templates[PROMPT_TYPE_SYS];
  const string_t *res_prompt_tpl = config->prompt_templates[PROMPT_TYPE_RES];

  strFmt(self->prompt, sys_prompt_tpl->data, NARRATOR_END_GAME_SYS_PROMPT.data);
  strFmtAppend(self->prompt, "\n ENDGAME: %s", world->end_game[state]);
  strFmtAppend(self->prompt, res_prompt_tpl->data, "");

  generateAndValidate(self->ai, self->prompt, description, &ACTION_MUST_HAVES);
}

void dmDestroy(dm_t **self) {
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
