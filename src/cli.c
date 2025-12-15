#include "cli.h"
#include "lib/buffers.h"
#include "linenoise.h"
#include "log.h"
#include "utils.h"
#include "world/command.h"
#include <stdio.h>
#include <stdlib.h>

static const char session_filename[] = ".ttyny";
static char session_path[256] = {};

static void completion(const char *buf, linenoiseCompletions *lc) {
  if (buf[0] == '/') {
    for (size_t i = 0; i < COMMAND_TYPES; i++) {
      if (strncmp(buf, command_names[i]->data, strlen(buf)) == 0) {
        linenoiseAddCompletion(lc, command_names[i]->data);
      }
    }
  }
}

void cliPromptInit(void) {
  char *home = getenv("HOME");
  if (home) {
    snprintf(session_path, sizeof(session_path), "%s/%s", home,
             session_filename);
  }
  linenoiseHistorySetMaxLen(256);
  linenoiseSetCompletionCallback(completion);
}

cli_readline_result_t cliReadline(string_t *input) {
  char *line = linenoise("> ");

  // This is invoked on Ctrl+C/D
  if (!line) {
    return CLI_READLINE_RESULT_QUIT;
  }

  strFmt(input, "%s", line);
  linenoiseFree(line);

  if (bufIsEmpty(input))
    return CLI_READLINE_RESULT_EMPTY;

  linenoiseHistoryAdd(input->data);
  if (session_path[0] != 0) {
    linenoiseHistorySave(session_path);
  }

  return CLI_READLINE_RESULT_OK;
}

void cliPrintUsageAndExit(void) {
  fprintf(stderr,
          "%s is a small-language-model-powered game engine to play text "
          "adventure games in your terminal.\n"
          "Usage:\n"
          "  %s [flags] <path-to-story.json>\n"
          "\n"
          "Flags:\n"
          "  -h, --help         show this help\n"
          "  -v, --version      show version\n"
          "  -l, --log=<level>  log level (default: error)\n"
          "\n"
          "For more information https://github.com/shikaan/%s\n",
          NAME_NO_TTY, NAME_NO_TTY, NAME_NO_TTY);
  exit(1);
}

static log_level_t parseLogLevel(const char *arg, unsigned long len) {
  const unsigned long diff = (arg[len] == '=') ? len + 1 : len;

  if (!strcmp(arg + diff, "info")) {
    return LOG_LEVEL_INFO;
  }
  if (!strcmp(arg + diff, "debug")) {
    return LOG_LEVEL_DEBUG;
  }
  if (!strcmp(arg + diff, "error")) {
    return LOG_LEVEL_ERROR;
  }

  char buffer[256];
  snprintf(buffer, sizeof(buffer),
           "unrecognized log level '%s'. Expected: debug, info, error.",
           arg + diff);

  cliPrintError(buffer);
  cliPrintUsageAndExit();
  return LOG_LEVEL_UNKNOWN;
}

void cliParseArgs(int argc, char **argv, cli_args_t *args) {
  if (argc < 2 || argc > 3) {
    cliPrintUsageAndExit();
  }

  if (argc == 2) {
    const char *arg = argv[argc - 1];

    if (!strcmp(arg, "-h") || !strcmp(arg, "--help")) {
      cliPrintUsageAndExit();
    }

    if (!strcmp(arg, "-v") || !strcmp(arg, "--version")) {
      fprintf(stderr, "%s - %s (%s)\n", NAME_NO_TTY, VERSION, SHA);
      exit(1);
    }

    if (!strncmp(arg, "-l", 2) || !strncmp(arg, "--log", 5)) {
      cliPrintError("missing story");
      cliPrintUsageAndExit();
    }

    if (arg[0] == '-') {
      cliPrintError("unrecognized flag");
      cliPrintUsageAndExit();
    }

    args->log_level = LOG_LEVEL_ERROR;
    args->story_path = arg;
    return;
  }

  if (argc == 3) {
    args->log_level = LOG_LEVEL_ERROR;
    args->story_path = argv[argc - 1];

    for (int i = 1; i < argc - 1; i++) {
      const char *arg = argv[i];

      if (!strcmp(arg, "-h") || !strcmp(arg, "--help")) {
        cliPrintUsageAndExit();
      }

      if (!strcmp(arg, "-v") || !strcmp(arg, "--version")) {
        fprintf(stderr, "%s - %s (%s)\n", NAME_NO_TTY, VERSION, SHA);
        exit(1);
      }

      if (!strncmp(arg, "-l", 2) || !strncmp(arg, "--log", 5)) {
        args->log_level = parseLogLevel(arg, strlen(arg));
        continue;
      }

      // unrecognised parameter
      cliPrintUsageAndExit();
    }
  }
}

void cliPrintError(const char *msg) {
  fprintf(stderr, "%s: %s\n", NAME_NO_TTY, msg);
}
