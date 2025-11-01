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
    {"eat bread", &ACTION_USE_NAME},
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
  const int is_command = bufAt(input, 0) == '/';

  if (is_command) {
    if (input->used > 1) {
      if (strStartsWith(&ACTION_HELP_NAME, input)) {
        return ACTION_HELP;
      } else if (strStartsWith(&ACTION_STATUS_NAME, input)) {
        return ACTION_STATUS;
      } else if (strStartsWith(&ACTION_QUIT_NAME, input)) {
        return ACTION_QUIT;
      }
    }

    return ACTION_UNKNOWN;
  }

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

static inline void parserExtractTarget(parser_t *self, const string_t *input,
                                       const locations_t *locations,
                                       const items_t *items,
                                       location_t **result_location,
                                       item_t **result_item) {
  panicif(!locations, "missing locations");
  panicif(!items, "missing items");
  panicif(!input, "missing input");

  const config_t *config = self->ai->configuration;
  const string_t *sys_prompt_tpl = config->prompt_templates[PROMPT_TYPE_SYS];
  const string_t *usr_prompt_tpl = config->prompt_templates[PROMPT_TYPE_USR];
  const string_t *res_prompt_tpl = config->prompt_templates[PROMPT_TYPE_RES];

  strFmt(self->prompt, sys_prompt_tpl->data, PARSER_TARGET_SYS_PROMPT.data);

  strFmt(self->target_grammar, "root ::= \"unknown\"");

  for (size_t i = 0; i < locations->used; i++) {
    location_t *exit = (location_t *)bufAt(locations, i);
    strFmtAppend(self->target_grammar, " | \"%s\"", exit->object.name);
  }

  for (size_t i = 0; i < items->used; i++) {
    item_t *item = bufAt(items, i);
    strFmtAppend(self->target_grammar, " | \"%s\"", item->object.name);
  }

  // Populate few-shots examples
  char shot_buffer[256] = {};
  if (locations->used) {
    for (size_t i = 0; i < arrLen(location_shots_tpls); i++) {
      const char *shot_tpl = location_shots_tpls[i];
      const size_t j = i % locations->used;
      location_t *exit = (location_t *)bufAt(locations, j);
      snprintf(shot_buffer, sizeof(shot_buffer), shot_tpl, exit->object.name);
      strFmtAppend(self->prompt, usr_prompt_tpl->data, shot_buffer);
      strFmtAppend(self->prompt, res_prompt_tpl->data, exit->object.name);
    }
  }

  if (items->used) {
    for (size_t i = 0; i < arrLen(item_shots_tpls); i++) {
      const char *shot_tpl = item_shots_tpls[i];
      const size_t j = i % items->used;
      const item_t *item = bufAt(items, j);
      snprintf(shot_buffer, sizeof(shot_buffer), shot_tpl, item->object.name);
      strFmtAppend(self->prompt, usr_prompt_tpl->data, shot_buffer);
      strFmtAppend(self->prompt, res_prompt_tpl->data, item->object.name);
    }
  }

  strFmtAppend(self->prompt, usr_prompt_tpl->data, input->data);
  strFmtAppend(self->prompt, res_prompt_tpl->data, "");

  aiSetGrammar(self->ai, self->target_grammar);
  strClear(self->response);
  aiGenerate(self->ai, self->prompt, self->response);
  strTrim(self->response);

  for (size_t i = 0; i < locations->used; i++) {
    location_t *location = (location_t *)bufAt(locations, i);
    if (objectIdEq(self->response->data, location->object.name)) {
      *result_item = NULL;
      *result_location = location;
      return;
    }
  }

  for (size_t i = 0; i < items->used; i++) {
    item_t *item = bufAt(items, i);
    if (objectIdEq(self->response->data, item->object.name)) {
      *result_item = item;
      *result_location = NULL;
      return;
    }
  }

  *result_item = NULL;
  *result_location = NULL;
}

static inline void parserDestroy(parser_t **self) {
  if (!self || !*self)
    return;

  aiDestroy(&(*self)->ai);
  strDestroy(&(*self)->prompt);
  strDestroy(&(*self)->response);
  strDestroy(&(*self)->target_grammar);
  deallocate(self);
}
