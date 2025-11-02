#include "screen.h"
#include "alloc.h"
#include "buffers.h"
#include "world.h"
#include <stddef.h>
#include <stdio.h>

static void sleep_ms(unsigned ms) {
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (long)(ms % 1000) * 1000000L;
  nanosleep(&ts, NULL);
}

static void *loading(void *args) {
  ui_handle_t *state = (ui_handle_t *)args;
  const char *text_variants[] = {"Thinking", "Hallucinating a bit of lore",
                                 "Almost there", "Still working",
                                 "Crafting some slop"};
  const char *dot_variants[] = {".  ", ".. ", "..."};
  for (size_t i = 0; !state->stop; i++) {
    sleep_ms(200);
    const char *dots = dot_variants[i % 3];
    const char *text = text_variants[(i / 18) % 5];
    printf("\033[2K\r%s%s", text, dots);
    fflush(stdout);
  }
  printf("\033[2K\r");
  fflush(stdout);
  return NULL;
}

ui_handle_t *loadingStart(void) {
  ui_handle_t *handle = allocate(sizeof(ui_handle_t));
  panicif(!handle, "cannot allocate loader");

  handle->stop = 0;
  pthread_t tid;
  pthread_create(&tid, NULL, loading, handle);
  handle->tid = tid;
  return handle;
}

void loadingStop(ui_handle_t **handle) {
  if (!(*handle) || !handle) return;
  (*handle)->stop = 1;
  pthread_join((*handle)->tid, NULL);
  deallocate(handle);
}

static void printResponse(string_t *response, const char *prefix) {
  const char *s = response->data;
  size_t col = 0;
  size_t prefix_len = strlen(prefix);

  // Always start with prefix
  fwrite(prefix, 1, prefix_len, stdout);
  col = prefix_len;

  while (*s) {
    if (*s == '\n') {
      putchar('\n');
      fwrite(prefix, 1, prefix_len, stdout);
      col = prefix_len;
      s++;
      continue;
    }
    // Find the end of the next word
    const char *word = s;
    while (*word && *word != ' ' && *word != '\n')
      word++;
    size_t word_len = (size_t)(word - s);
    // If the word doesn't fit, break line
    if (col > prefix_len && col + word_len > 80) {
      putchar('\n');
      fwrite(prefix, 1, prefix_len, stdout);
      col = prefix_len;
    }
    // Print the word
    fwrite(s, 1, word_len, stdout);
    col += word_len;
    s = word;
    // Print the space if present
    if (*s == ' ') {
      putchar(' ');
      col++;
      s++;
    }
  }
  putchar('\n');
}

void printError(string_t *response) { printResponse(response, " ! "); }
void printStateUpdate(string_t *response) {
  puts("");
  printResponse(response, " ~> ");
}
void printCommandOutput(string_t *response) {
  printResponse(response, " ~   ");
}
void printDescription(string_t *response) { printResponse(response, " |  "); }
void printPrompt(void) { printf("> "); }

void screenClear(void) { puts("\e[1;1H\e[2J"); }

void printWelcomeScreen(const char *name, string_t *response) {
  screenClear();
  strFmt(response,
         "Welcome to %s!\n"
         "\n"
         "You're about to explore a fantasy world through conversation.\n"
         "The game will describe where you are, and you respond in your own "
         "words.\n"
         "\n"
         "Just type one action at a time naturally, like:\n"
         "  • %s\n"
         "  • %s\n"
         "  • %s\n"
         "\n"
         "Type %s anytime, if you're stuck.\n"
         "\n"
         "[Press ENTER to begin your adventure]",
         name, prompt("I want to go in the kitchen"),
         prompt("Pick up the lamp"), prompt("Open this door"),
         command("/help"));
  printCommandOutput(response);
  fgetc(stdin);
  screenClear();
}

void printLocationChange(string_t *response, location_t *loc) {
  strFmt(response, "Location: " location("%s"), loc->object.name);
  printStateUpdate(response);
}
