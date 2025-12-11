#pragma once

#include "lib/buffers.h"

typedef struct {
  const char* story_path;
} cli_args_t;

typedef enum {
  CLI_READLINE_RESULT_OK,
  CLI_READLINE_RESULT_EMPTY,
  CLI_READLINE_RESULT_QUIT,
} cli_readline_result_t;

// Initialize the CLI interface
void cliPromptInit(void);
cli_readline_result_t cliReadline(string_t *);

void cliPrintError(const char *);
void cliPrintUsageAndExit(void);

void cliParseArgs(int, char**, cli_args_t*);
