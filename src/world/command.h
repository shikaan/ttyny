#pragma once

#include "../lib/buffers.h"

typedef enum {
  COMMAND_TYPE_UNKNOWN = -1,
  COMMAND_TYPE_HELP = 0,
  COMMAND_TYPE_STATUS,
  COMMAND_TYPE_QUIT,
  COMMAND_TYPE_TLDR,

  COMMAND_TYPES
} command_type_t;

static string_t COMMAND_HELP = strConst("/help");
static string_t COMMAND_STATUS = strConst("/status");
static string_t COMMAND_QUIT = strConst("/quit");
static string_t COMMAND_TLDR = strConst("/tldr");

static string_t *command_names[COMMAND_TYPES] = {&COMMAND_HELP, &COMMAND_STATUS,
                                                 &COMMAND_QUIT, &COMMAND_TLDR};
