#include "parser.h"
#include "configs/qwen.h"
#include "lib/panic.h"
#include "utils.h"
#include <stddef.h>

static string_t ACTION_GRAMMAR = strConst(
    "root ::= \"move\" | \"use\" | \"take\" | \"drop\" | \"examine\"\n");

typedef struct {
  const char *input;
  const string_t *output;
} action_shot_t;

static action_shot_t action_shots[] = {
    {"examine book", &ACTION_EXAMINE},
    {"take apple", &ACTION_TAKE},
    {"move to garden", &ACTION_MOVE},
    {"use knife", &ACTION_USE},
    {"drop the ball", &ACTION_DROP},

    {"look at the key", &ACTION_EXAMINE},
    {"grab the sword", &ACTION_TAKE},
    {"walk to kitchen", &ACTION_MOVE},
    {"use crowbar on chest", &ACTION_USE},
    {"put down the cup", &ACTION_DROP},

    {"check behind painting", &ACTION_EXAMINE},
    {"pick up coin", &ACTION_TAKE},
    {"head north", &ACTION_MOVE},
    {"eat bread", &ACTION_USE},
    {"throw away rock", &ACTION_DROP},
};

static const char *item_shots_tpls[] = {"look at the %s", "grab the %s",
                                        "use %s on chest", "check behind %s",
                                        "pick up %s"};

static const char *location_shots_tpls[] = {
    "walk to %s", "enter %s", "go to %s", "move to %s", "run towards %s"};

parser_t *parserCreate(void) {
  parser_t *parser = allocate(sizeof(parser_t));
  panicif(!parser, "cannot allocate parser");

  ai_result_t result;
  parser->ai = aiCreate(&PARSER_CONFIG, &result);
  panicif(!parser->ai, "cannot allocate AI for parser");

  parser->prompt = strCreate(4096);
  panicif(!parser->prompt, "cannot allocate prompt buffer");

  parser->response = strCreate(128);
  panicif(!parser->response, "cannot allocate response buffer");

  parser->target_grammar = strCreate(4096);
  panicif(!parser->target_grammar, "cannot allocate grammar buffer");

  return parser;
}

void parserGetOperation(parser_t *self, operation_t *operation,
                        const string_t *input) {
  const int is_command = bufAt(input, 0) == '/';

  if (is_command) {
    operation->type = OPERATION_TYPE_COMMAND;
    operation->as.command = COMMAND_TYPE_UNKNOWN;

    if (input->len <= 1)
      return;

    if (strStartsWith(&COMMAND_HELP, input)) {
      operation->as.command = COMMAND_TYPE_HELP;
      return;
    } else if (strStartsWith(&COMMAND_STATUS, input)) {
      operation->as.command = COMMAND_TYPE_STATUS;
      return;
    } else if (strStartsWith(&COMMAND_QUIT, input)) {
      operation->as.command = COMMAND_TYPE_QUIT;
      return;
    } else if (strStartsWith(&COMMAND_TLDR, input)) {
      operation->as.command = COMMAND_TYPE_TLDR;
      return;
    }

    return;
  }

  operation->type = OPERATION_TYPE_ACTION;
  operation->as.action = ACTION_TYPE_UNKNOWN;

  const config_t *config = self->ai->configuration;
  const string_t *sys_prompt_tpl = config->prompt_templates[PROMPT_TYPE_SYS];
  const string_t *usr_prompt_tpl = config->prompt_templates[PROMPT_TYPE_USR];
  const string_t *res_prompt_tpl = config->prompt_templates[PROMPT_TYPE_RES];

  strFmt(self->prompt, sys_prompt_tpl->data, PARSER_ACTION_SYS_PROMPT.data);

  for (size_t i = 0; i < arrLen(action_shots); i++) {
    action_shot_t shot = action_shots[i];
    strFmtAppend(self->prompt, usr_prompt_tpl->data, shot.input);
    strFmtAppend(self->prompt, res_prompt_tpl->data, shot.output->data);
  }

  strFmtAppend(self->prompt, usr_prompt_tpl->data, input->data);
  strFmtAppend(self->prompt, res_prompt_tpl->data, "");

  ai_result_t result = aiSetGrammar(self->ai, &ACTION_GRAMMAR);
  panicif(result != AI_RESULT_OK, "cannot set grammar");
  strClear(self->response);
  result = aiGenerate(self->ai, self->prompt, self->response);
  panicif(result != AI_RESULT_OK, "cannot generate response");

  for (size_t i = 0; i < ACTION_TYPES; i++) {
    if (strEq(self->response, action_names[i])) {
      operation->as.action = actions_types[i];
      return;
    }
  }
}

void parserExtractTarget(parser_t *self, const string_t *input,
                         const locations_t *locations, const items_t *items,
                         location_t **result_location, item_t **result_item) {
  panicif(!locations, "missing locations");
  panicif(!items, "missing items");
  panicif(!input, "missing input");

  const config_t *config = self->ai->configuration;
  const string_t *sys_prompt_tpl = config->prompt_templates[PROMPT_TYPE_SYS];
  const string_t *usr_prompt_tpl = config->prompt_templates[PROMPT_TYPE_USR];
  const string_t *res_prompt_tpl = config->prompt_templates[PROMPT_TYPE_RES];

  strFmt(self->prompt, sys_prompt_tpl->data, PARSER_TARGET_SYS_PROMPT.data);

  strFmt(self->target_grammar, "root ::= \"unknown\"");

  size_t i = 0;
  bufEach(locations, i) {
    location_t *exit = bufAt(locations, i);
    strFmtAppend(self->target_grammar, " | \"%s\"", exit->object.name);
  }

  bufEach(items, i) {
    item_t *item = bufAt(items, i);
    strFmtAppend(self->target_grammar, " | \"%s\"", item->object.name);
  }

  char shot_buffer[256] = {};
  if (locations->len) {
    for (i = 0; i < arrLen(location_shots_tpls); i++) {
      const char *shot_tpl = location_shots_tpls[i];
      const size_t j = i % locations->len;
      location_t *exit = (location_t *)bufAt(locations, j);
      snprintf(shot_buffer, sizeof(shot_buffer), shot_tpl, exit->object.name);
      strFmtAppend(self->prompt, usr_prompt_tpl->data, shot_buffer);
      strFmtAppend(self->prompt, res_prompt_tpl->data, exit->object.name);
    }
  }

  if (items->len) {
    for (i = 0; i < arrLen(item_shots_tpls); i++) {
      const char *shot_tpl = item_shots_tpls[i];
      const size_t j = i % items->len;
      const item_t *item = bufAt(items, j);
      snprintf(shot_buffer, sizeof(shot_buffer), shot_tpl, item->object.name);
      strFmtAppend(self->prompt, usr_prompt_tpl->data, shot_buffer);
      strFmtAppend(self->prompt, res_prompt_tpl->data, item->object.name);
    }
  }

  strFmtAppend(self->prompt, usr_prompt_tpl->data, input->data);
  strFmtAppend(self->prompt, res_prompt_tpl->data, "");

  ai_result_t result = aiSetGrammar(self->ai, self->target_grammar);
  panicif(result != AI_RESULT_OK, "cannot set grammar");
  strClear(self->response);
  result = aiGenerate(self->ai, self->prompt, self->response);
  panicif(result != AI_RESULT_OK, "cannot generate response");
  strTrim(self->response);

  bufEach(locations, i) {
    location_t *location = bufAt(locations, i);
    if (objectNameEq(self->response->data, location->object.name)) {
      debug("found location: %s", location->object.name);
      *result_item = NULL;
      *result_location = location;
      return;
    }
  }

  bufEach(items, i) {
    item_t *item = bufAt(items, i);
    if (objectNameEq(self->response->data, item->object.name)) {
      debug("found item: %s", item->object.name);
      *result_item = item;
      *result_location = NULL;
      return;
    }
  }

  *result_item = NULL;
  *result_location = NULL;
}

void parserDestroy(parser_t **self) {
  if (!self || !*self)
    return;

  aiDestroy(&(*self)->ai);
  strDestroy(&(*self)->prompt);
  strDestroy(&(*self)->response);
  strDestroy(&(*self)->target_grammar);
  deallocate(self);
}
