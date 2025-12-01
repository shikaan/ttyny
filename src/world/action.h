#pragma once

#include "../lib/buffers.h"

typedef enum {
  ACTION_TYPE_UNKNOWN = -1,
  ACTION_TYPE_MOVE = 0,
  ACTION_TYPE_TAKE,
  ACTION_TYPE_DROP,
  ACTION_TYPE_USE,
  ACTION_TYPE_EXAMINE,

  ACTION_TYPES,
} action_type_t;

static string_t ACTION_MOVE = strConst("move");
static string_t ACTION_USE = strConst("use");
static string_t ACTION_TAKE = strConst("take");
static string_t ACTION_DROP = strConst("drop");
static string_t ACTION_EXAMINE = strConst("examine");

static action_type_t actions_types[ACTION_TYPES] = {
    ACTION_TYPE_MOVE, ACTION_TYPE_TAKE, ACTION_TYPE_DROP, ACTION_TYPE_USE,
    ACTION_TYPE_EXAMINE};

static string_t *action_names[ACTION_TYPES] = {
    &ACTION_MOVE, &ACTION_TAKE, &ACTION_DROP, &ACTION_USE, &ACTION_EXAMINE};
