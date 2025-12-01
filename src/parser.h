#pragma once

#include "ai.h"
#include "lib/buffers.h"
#include "world/action.h"
#include "world/command.h"
#include "world/item.h"
#include "world/location.h"

#include <stddef.h>

typedef struct {
  ai_t *ai;
  string_t *prompt;
  string_t *response;
  string_t *target_grammar;
} parser_t;

typedef enum {
  OPERATION_TYPE_ACTION,
  OPERATION_TYPE_COMMAND,

  OPERATION_TYPES
} operation_type_t;

typedef struct {
  operation_type_t type;
  union {
    action_type_t action;
    command_type_t command;
  } as;
} operation_t;

parser_t *parserCreate(void);

void parserGetOperation(parser_t*, operation_t*, const string_t*);

void parserExtractTarget(parser_t *, const string_t *, const locations_t *,
                         const items_t *, location_t **, item_t **);

void parserDestroy(parser_t **self);
