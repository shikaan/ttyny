#include "cli.h"
#include "linenoise.h"
#include "utils.h"
#include "world/command.h"

void completion(const char *buf, linenoiseCompletions *lc) {
  if (buf[0] == '/') {
    for (size_t i = 0; i < COMMAND_TYPES; i++) {
      if (strncmp(buf, command_names[i]->data, strlen(buf)) == 0) {
        linenoiseAddCompletion(lc, command_names[i]->data);
      }
    }
  }
}

void cliPromptInit(void) {
  linenoiseHistorySetMaxLen(15);
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

  return CLI_READLINE_RESULT_OK;
}

void cliPrintUsageAndExit(void) {
  fprintf(stderr,
          "%s is a small-language-model-powered game engine to play text "
          "adventure games in your terminal.\n"
          "Usage:\n"
          "  %s <path-to-story.json>\n"
          "\n"
          "Flags:\n"
          "  -h, --help      show this help\n"
          "  -v, --version   show version\n"
          "\n"
          "For more information https://github.com/shikaan/%s\n",
          NAME_NO_TTY, NAME_NO_TTY, NAME_NO_TTY);
  exit(1);
}

void cliParseArgs(int argc, char **argv, cli_args_t *args) {
  if (argc != 2) {
    cliPrintUsageAndExit();
  }

  const char* arg = argv[argc - 1];

  if (!strcmp(arg, "-h") || !strcmp(arg, "--help")) {
    return cliPrintUsageAndExit();
  }

  if (!strcmp(arg, "-v") || !strcmp(arg, "--version")) {
    fprintf(stderr, "%s - %s (%s)\n", NAME_NO_TTY, VERSION, SHA);
    exit(1);
    return;
  }

  args->story_path = argv[argc - 1];
}

void cliPrintError(const char *msg) {
  fprintf(stderr, "%s: %s\n", NAME_NO_TTY, msg);
}
