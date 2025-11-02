#include "ai.h"
#include "alloc.h"
#include "buffers.h"
#include "utils.h"
#include "world.h"

#include "configs/qwen.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  ai_t *ai;
  string_t *prompt;
  string_t *summary;
} dm_t;

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

words_t STOP_WORDS =
    bufConst(6, "item", "items", "inventory", "player", "player's", "location");
words_t STOP_WORDS_CASE = bufConst(2, "EXITS", "EXIT");
words_t STOP_CHARS = bufConst(2, "[", "(");
words_t ACTION_MUST_HAVES = bufConst(1, "you");

static failure_shot_t failure_shots[] = {
    {"look at the key", FAILURE_TYPE_INVALID_TARGET,
     "You can't see any such thing."},
    {"inspect banana", FAILURE_TYPE_INVALID_TARGET,
     "A banana? The area is distinctly banana-free."},
    {"enter the closet", FAILURE_TYPE_INVALID_LOCATION,
     "The closet, pleasant as it may be, is not an exit."},
    {"enter the bottle", FAILURE_TYPE_INVALID_LOCATION,
     "You'd have to be considerably smaller to fit in there."},
    {"take mountain", FAILURE_TYPE_CANNOT_COLLECT_ITEM,
     "You try to take it, but it won't budge."},
    {"take horizon", FAILURE_TYPE_CANNOT_COLLECT_ITEM,
     "It politely remains where it is."},
    {"use phantom wrench", FAILURE_TYPE_INVALID_ITEM, "You lack such a thing."},
    {"drop imaginary gem", FAILURE_TYPE_INVALID_ITEM,
     "Hard to drop what you don't possess."},
    {"use plain wall", FAILURE_TYPE_CANNOT_BE_USED,
     "The wall is unimpressed by your efforts."},
    {"use decorative plant", FAILURE_TYPE_CANNOT_BE_USED,
     "It offers ambiance, not functionality."},
};

static void summarize(const world_t *world, string_t *summary) {
  location_t *current_location = world->current_location;

  strFmt(summary, "LOCATION: %s (%s) [%s]\n", current_location->object.name,
         current_location->object.description,
         bufAt(current_location->object.state_descriptions,
               current_location->object.current_state));

  if (world->state.inventory->used) {
    strFmtAppend(summary, "INVENTORY:\n");
    for (size_t i = 0; i < world->state.inventory->used; i++) {
      item_t *item = bufAt(world->state.inventory, i);
      strFmtAppend(
          summary, "  - %s (%s) [%s]\n", item->object.name,
          item->object.description,
          bufAt(item->object.state_descriptions, item->object.current_state));
    }
  }

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

dm_t *dmCreate(void) {
  dm_t *narrator = allocate(sizeof(dm_t));
  panicif(!narrator, "cannot allocate narrator");

  ai_result_t result;
  narrator->ai = aiCreate(&NARRATOR_CONFIG, &result);
  panicif(!narrator->ai, "cannot allocate AI for narrator");

  narrator->prompt = strCreate(4096);
  panicif(!narrator->prompt, "cannot allocate prompt buffer");

  narrator->summary = strCreate(4096);
  panicif(!narrator->summary, "cannot allocate summary buffer");

  return narrator;
}

void dmDestroy(dm_t **self) {
  if (!self || !*self)
    return;

  aiDestroy(&(*self)->ai);
  strDestroy(&(*self)->prompt);
  strDestroy(&(*self)->summary);
  deallocate(self);
}

static int hasStopWords(string_t *response) {
  panicif(!response, "missing response");

  string_t *input cleanup(strDestroy) = strDup(response);
  if (!input)
    return 0;

  char *saveptr = NULL;
  char *token = strtok_r(input->data, " \t\r\n", &saveptr);
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

    token = strtok_r(NULL, " \t\r\n", &saveptr);
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
  while (!valid) {
    strClear(response);
    aiReset(ai, &result);
    panicif(result != AI_RESULT_OK, "cannot reset model state");
    aiGenerate(ai, &result, prompt, response);
    panicif(result != AI_RESULT_OK, "cannot generate response");
    valid = !hasStopWords(response) && hasAllMustHaves(response, must_haves);
  }
}

void dmDescribeWorld(dm_t *self, const world_t *world, string_t *description) {
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
}

void dmDescribeObject(dm_t *self, object_t *object, string_t *description) {
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
}

void dmDescribeFail(dm_t *self, failure_type_t failure, string_t *input,
                    string_t *comment) {
  const config_t *config = self->ai->configuration;
  const string_t *sys_prompt_tpl = config->prompt_templates[PROMPT_TYPE_SYS];
  const string_t *usr_prompt_tpl = config->prompt_templates[PROMPT_TYPE_USR];
  const string_t *res_prompt_tpl = config->prompt_templates[PROMPT_TYPE_RES];

  strFmt(self->prompt, sys_prompt_tpl->data, NARRATOR_FAILURE_SYS_PROMPT.data);

  const char *tpl = "ACTION: %s\n"
                    "FAILURE: %s";

  char shot_buffer[256] = {};
  for (size_t i = 0; i < arrLen(failure_shots); i++) {
    const failure_shot_t shot = failure_shots[i];
    snprintf(shot_buffer, sizeof(shot_buffer), tpl, shot.action,
             failure_names[shot.failure]->data);
    strFmtAppend(self->prompt, usr_prompt_tpl->data, shot_buffer);
    strFmtAppend(self->prompt, res_prompt_tpl->data, shot.response);
  }

  strFmt(self->summary, tpl, input->data, failure_names[failure]->data);
  strFmtAppend(self->prompt, usr_prompt_tpl->data, self->summary->data);
  strFmtAppend(self->prompt, res_prompt_tpl->data, "");

  generateAndValidate(self->ai, self->prompt, comment, &ACTION_MUST_HAVES);
}

void dmDescribeSuccess(dm_t *self, world_t *world, const string_t *input,
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

void dmNarrateEndGame(dm_t *self, world_t *world, game_state_t state,
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
