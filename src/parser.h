#pragma once

#include "ai.h"
#include "alloc.h"
#include "buffers.h"
#include "world.h"

#include "configs/qwen.h"
#include <stddef.h>

typedef struct {
  ai_t *ai;
  string_t *prompt;
  string_t *response;
  string_t *target_grammar;
} parser_t;

// TODO: should this have an "unknown" too?
static string_t ACTION_GRAMMAR = strConst(
    "root ::= \"move\" | \"use\" | \"take\" | \"drop\" | \"examine\"\n");

typedef struct {
  const char *input;
  const string_t *output;
} action_shot_t;

static action_shot_t action_shots[] = {
    {"look at the key", &ACTION_EXAMINE_NAME},
    {"grab the sword", &ACTION_TAKE_NAME},
    {"walk to kitchen", &ACTION_MOVE_NAME},
    {"use crowbar on chest", &ACTION_USE_NAME},
    {"check behind painting", &ACTION_EXAMINE_NAME},
    {"pick up coin", &ACTION_TAKE_NAME},
    {"head north", &ACTION_MOVE_NAME},
};

static const char *item_shots_tpls[] = {
    "look at the %s",  "grab the %s", "use %s on chest",
    "check behind %s", "pick up %s",
};

static const char *location_shots_tpls[] = {
    "walk to %s", "enter %s", "go to %s", "move to %s", "run towards %s",
};

static inline parser_t *parserCreate(void) {
  parser_t *parser = allocate(sizeof(parser_t));
  panicif(!parser, "cannot allocate parser");

  parser->ai = aiCreate(&PARSER_CONFIG);
  panicif(!parser->ai, "cannot allocate AI for parser");

  parser->prompt = strCreate(4096);
  panicif(!parser->prompt, "cannot allocate prompt buffer");

  parser->response = strCreate(128);
  panicif(!parser->response, "cannot allocate response buffer");

  parser->target_grammar = strCreate(4096);
  panicif(!parser->target_grammar, "cannot allocate grammar buffer");

  return parser;
}

static inline action_t parserExtractAction(parser_t *self,
                                           const string_t *input) {
  const config_t *config = self->ai->configuration;
  const string_t *sys_prompt_tpl = config->prompt_templates[PROMPT_TYPE_SYS];
  const string_t *usr_prompt_tpl = config->prompt_templates[PROMPT_TYPE_USR];
  const string_t *res_prompt_tpl = config->prompt_templates[PROMPT_TYPE_RES];

  strFmt(self->prompt, sys_prompt_tpl->data, PARSER_ACTION_SYS_PROMPT.data);

  // Populate few-shots examples
  for (size_t i = 0; i < arrLen(action_shots); i++) {
    action_shot_t shot = action_shots[i];
    strFmtAppend(self->prompt, usr_prompt_tpl->data, shot.input);
    strFmtAppend(self->prompt, res_prompt_tpl->data, shot.output->data);
  }

  strFmtAppend(self->prompt, usr_prompt_tpl->data, input->data);
  strFmtAppend(self->prompt, res_prompt_tpl->data, "");

  aiSetGrammar(self->ai, &ACTION_GRAMMAR);
  strClear(self->response);
  aiGenerate(self->ai, self->prompt, self->response);

  for (size_t i = 0; i < ACTIONS; i++) {
    if (strEq(self->response, action_names[i])) {
      return actions_types[i];
    }
  }

  return ACTION_UNKNOWN;
}

static inline object_t *parserExtractTarget(parser_t *self,
                                            const string_t *input,
                                            const world_t *world) {
  const config_t *config = self->ai->configuration;
  const string_t *sys_prompt_tpl = config->prompt_templates[PROMPT_TYPE_SYS];
  const string_t *usr_prompt_tpl = config->prompt_templates[PROMPT_TYPE_USR];
  const string_t *res_prompt_tpl = config->prompt_templates[PROMPT_TYPE_RES];

  strFmt(self->prompt, sys_prompt_tpl->data, PARSER_TARGET_SYS_PROMPT.data);

  strFmt(self->target_grammar, "root ::= \"unknown\"");

  const locations_t *exits = world->current_location->exits;
  for (size_t i = 0; i < exits->used; i++) {
    location_t *exit = (location_t *)bufAt(exits, i);
    strFmtAppend(self->target_grammar, " | \"%s\"", exit->object.name);
  }

  const items_t *items = world->current_location->items;
  for (size_t i = 0; i < items->used; i++) {
    item_t *item = bufAt(items, i);
    strFmtAppend(self->target_grammar, " | \"%s\"", item->object.name);
  }

  // Populate few-shots examples
  char shot_buffer[256] = {};
  for (size_t i = 0; i < arrLen(location_shots_tpls); i++) {
    const char *shot_tpl = location_shots_tpls[i];
    const size_t j = i % items->used;
    location_t *exit = (location_t *)bufAt(exits, j);
    snprintf(shot_buffer, sizeof(shot_buffer), shot_tpl, exit->object.name);
    strFmtAppend(self->prompt, usr_prompt_tpl->data, shot_buffer);
    strFmtAppend(self->prompt, res_prompt_tpl->data, exit->object.name);
  }

  for (size_t i = 0; i < arrLen(item_shots_tpls); i++) {
    const char *shot_tpl = item_shots_tpls[i];
    const size_t j = i % items->used;
    const item_t *item = bufAt(items, j);
    snprintf(shot_buffer, sizeof(shot_buffer), shot_tpl, item->object.name);
    strFmtAppend(self->prompt, usr_prompt_tpl->data, shot_buffer);
    strFmtAppend(self->prompt, res_prompt_tpl->data, item->object.name);
  }

  strFmtAppend(self->prompt, usr_prompt_tpl->data, input->data);
  strFmtAppend(self->prompt, res_prompt_tpl->data, "");

  aiSetGrammar(self->ai, self->target_grammar);
  strClear(self->response);
  aiGenerate(self->ai, self->prompt, self->response);
  strTrim(self->response);

  for (size_t i = 0; i < exits->used; i++) {
    location_t *exit = (location_t *)bufAt(exits, i);
    if (objectIdEq(self->response->data, exit->object.name)) {
      return (object_t *)exit;
    }
  }

  for (size_t i = 0; i < items->used; i++) {
    item_t *item = bufAt(items, i);
    if (objectIdEq(self->response->data, item->object.name)) {
      return (object_t *)item;
    }
  }

  return NULL;
}

static inline void parserDestroy(parser_t **self) {
  if (!self || !*self)
    return;

  aiDestory(&(*self)->ai);
  strDestroy(&(*self)->prompt);
  strDestroy(&(*self)->response);
  strDestroy(&(*self)->target_grammar);
  deallocate(self);
}
