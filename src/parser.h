#pragma once

#include "ai.h"
#include "buffers.h"
#include "world.h"

#include <stddef.h>

typedef struct {
  ai_t *ai;
  string_t *prompt;
  string_t *response;
  string_t *target_grammar;
} parser_t;

parser_t *parserCreate(void);
action_type_t parserExtractAction(parser_t *self, const string_t *input);
void parserExtractTarget(parser_t *self, const string_t *input,
                         const locations_t *locations, const items_t *items,
                         location_t **result_location, item_t **result_item);
void parserDestroy(parser_t **self);

