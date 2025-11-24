#include "master.h"
#include "ai.h"
#include "alloc.h"
#include "buffers.h"
#include "map.h"
#include "utils.h"
#include "world/world.h"

#include "configs/qwen.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

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

typedef struct {
  const char *input;
  const char *output;
} action_desc_shot_t;

static action_desc_shot_t action_desc_shots[] = {
    {"ACTION: take apple\nTARGET: apple (A ripe red fruit)",
     "You pick up the apple."},
    {"ACTION: use knife\nTARGET: knife (A sharp steel blade)",
     "You brandish the knife, its blade gleaming."},
    {"ACTION: drop the ball\nTARGET: ball (A leather sphere)",
     "You drop the ball and it bounces away."},
};

static void summarizeLocation(const location_t *location, string_t *summary) {
  strFmt(summary,
         "LOCATION: %s\n"
         "DESCRIPTION: %s\n",
         location->object.name,
         bufAt(location->object.descriptions, location->object.state));

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

static const char *WORD_BREAK = " \t\r\n:-*'.,";

static int hasStopWords(string_t *response) {
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
  for (size_t i = 0; i < 20 && !valid; i++) {
    strClear(response);
    aiReset(ai, &result);
    panicif(result != AI_RESULT_OK, "cannot reset model state");
    aiGenerate(ai, &result, prompt, response);
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

static int objectIsReadable(const object_t *object) {
  if (!object || object->type != OBJECT_TYPE_ITEM)
    return 0;
  const item_t *item = (const item_t *)object;
  return item->readable;
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

  summarizeLocation(location, self->summary);
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

  if (objectIsReadable(object)) {
    const char *state_desc = bufAt(object->descriptions, object->state);
    strFmt(description, "%s reads:\n\n%s", object->name, state_desc);
    char *copy = strdup(description->data);
    (void)mapSet(self->descriptions, cache_key, copy);
    return;
  }

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
}

void masterDescribeAction(master_t *self, const world_t *world,
                          const string_t *input, const object_t *object,
                          string_t *comment) {
  const config_t *config = self->ai->configuration;
  const string_t *sys_prompt_tpl = config->prompt_templates[PROMPT_TYPE_SYS];
  const string_t *usr_prompt_tpl = config->prompt_templates[PROMPT_TYPE_USR];
  const string_t *res_prompt_tpl = config->prompt_templates[PROMPT_TYPE_RES];

  strFmt(self->prompt, sys_prompt_tpl->data, MASTER_ACTION_SYS_PROMPT.data);

  // Using a shot to provide some context
  strFmtAppend(self->prompt, usr_prompt_tpl->data, "look around");
  masterDescribeLocation(self, world->location, self->summary);
  strFmtAppend(self->prompt, res_prompt_tpl->data, self->summary->data);

  // // Add few-shot examples
  for (size_t i = 0; i < arrLen(action_desc_shots); i++) {
    action_desc_shot_t shot = action_desc_shots[i];
    strFmtAppend(self->prompt, usr_prompt_tpl->data, shot.input);
    strFmtAppend(self->prompt, res_prompt_tpl->data, shot.output);
  }

  strFmt(self->summary, "ACTION: %s\nTARGET:\n name: %s\n description: %s\n",
         input->data, object->name, bufAt(object->descriptions, object->state));
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

  strFmt(self->prompt, sys_prompt_tpl->data, MASTER_END_GAME_SYS_PROMPT.data);

  // Using a shot to provide some context
  strFmtAppend(self->prompt, usr_prompt_tpl->data, "look around");
  masterDescribeLocation(self, world->location, self->summary);
  strFmtAppend(self->prompt, res_prompt_tpl->data, self->summary->data);

  strFmt(self->summary, "ACTION: %s\nENDING: %s\nREASON: %s", last_action->data,
         state == GAME_STATE_VICTORY ? "victory" : "death", world->end_game);
  strFmtAppend(self->prompt, usr_prompt_tpl->data, self->summary->data);
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
  mapDestroy(&(*self)->descriptions);

  deallocate(self);
}
