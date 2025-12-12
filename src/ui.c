#include "ui.h"

#include "fmt.h"
#include "lib/tty.h"
#include "utils.h"

static const int SCREEN_WIDTH = 80;

static void sleep_ms(unsigned ms) {
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (long)(ms % 1000) * 1000000L;
  nanosleep(&ts, NULL);
}

static void *loading(void *args) {
  ui_handle_t *state = (ui_handle_t *)args;
  const char *text_variants[] = {
      "Loading",      "Thinking",      "Hallucinating a bit of lore",
      "Almost there", "Still working", "Crafting some slop"};
  const char *dot_variants[] = {".  ", ".. ", "..."};
  for (size_t i = 0; !state->stop; i++) {
    sleep_ms(200);
    const char *dots = dot_variants[i % 3];
    const char *text = text_variants[(i / 18) % 6];
    printf("\033[2K\r%s%s", text, dots);
    fflush(stdout);
  }
  printf("\033[2K\r");
  fflush(stdout);
  return NULL;
}

ui_handle_t *uiLoadingStart(void) {
  ui_handle_t *handle = allocate(sizeof(ui_handle_t));
  panicif(!handle, "cannot allocate loader");

  handle->stop = 0;
  pthread_t tid;
  pthread_create(&tid, NULL, loading, handle);
  handle->tid = tid;
  return handle;
}

void uiLoadingStop(ui_handle_t **handle) {
  if (!(*handle) || !handle)
    return;
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
      fwrite(ESC_RESET, 1, sizeof(ESC_RESET), stdout);
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
    if (col > prefix_len && col + word_len > SCREEN_WIDTH) {
      fwrite(ESC_RESET, 1, sizeof(ESC_RESET), stdout);
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
  fwrite(ESC_RESET, 1, sizeof(ESC_RESET), stdout);
  putchar('\n');
}

// Get visible string length, removing control sequences
static size_t strVisibleLength(const char *s) {
  size_t len = 0;
  size_t str_len = strlen(s);
  for (size_t i = 0; i < str_len - 1; i++) {
    char curr = s[i];
    char next = s[i + 1];
    if (curr == 0x1B && next == '[') {
      i += 2;
      while ((isdigit(s[i]) || s[i] == ';' || s[i] == '?' || s[i] == ':' ||
              s[i] == '<' || s[i] == '=' || s[i] == '>')) {
        i++;
      }
      continue;
    }

    len++;
  }
  return len;
}

// Print a string in the (horizontal) center of the screen
static void printCentered(const char *str) {
  size_t prefix_len = SCREEN_WIDTH / 2 - strVisibleLength(str) / 2;

  if (prefix_len < SCREEN_WIDTH) {
    for (size_t i = 0; i < prefix_len; i++) {
      putchar(' ');
    }
  }

  printf("%s\n", str);
}

void uiPrintError(string_t *response) {
  printResponse(response, failfmt(" !  "));
}

void uiPrintStateUpdates(strings_t *states) {
  if (!bufIsEmpty(states)) {
    printf("\n");
    size_t i;
    bufEach(states, i) {
      string_t *state = bufAt(states, i);
      printResponse(state, " ~> ");
    }
  }
}

void uiPrintCommandOutput(string_t *response) {
  printResponse(response, " ~  ");
}

void uiPrintDescription(string_t *response) { printResponse(response, " |  "); }

void uiPrintReadable(string_t *response) {
  printResponse(response, "    " ESC_ITALIC);
}

void uiFormatAndPrintEndGame(string_t *buffer, game_state_t state,
                    const world_t *world) {
  const char *state_text = state == GAME_STATE_VICTORY
                               ? successfmt("~~~>   YOU WON!   <~~~")
                               : failfmt("~~~>   GAME  OVER   <~~~");
  printCentered(state_text);

  printf("\n");
  strFmt(buffer, underline("STATS"));
  printCentered(buffer->data);
  printf("\n");

  size_t discovered_items = setUsed(world->discovered_items);
  strFmt(buffer, "Items: " numberfmt("%lu/%lu"), discovered_items,
         world->items->len);
  printCentered(buffer->data);

  size_t discovered_locations = setUsed(world->discovered_locations);
  strFmt(buffer, "Locations: " numberfmt("%lu/%lu"), discovered_locations,
         world->locations->len);
  printCentered(buffer->data);

  size_t total_puzzles = 0;
  size_t i = 0;
  bufEach(world->items, i) {
    item_t *item = bufAt(world->items, i);
    transitions_t *transitions = item->object.transitions;
    if (transitions && !bufIsEmpty(transitions)) {
      total_puzzles++;
    }
  }

  bufEach(world->locations, i) {
    location_t *location = bufAt(world->locations, i);
    transitions_t *transitions = location->object.transitions;
    if (transitions && !bufIsEmpty(transitions)) {
      total_puzzles++;
    }
  }

  size_t solved_puzzles = setUsed(world->solved_puzzles);
  strFmt(buffer, "Puzzles: " numberfmt("%lu/%lu"), solved_puzzles,
         total_puzzles);
  printCentered(buffer->data);

  size_t actual = discovered_locations + discovered_items + solved_puzzles;
  size_t total = total_puzzles + world->locations->len + world->items->len;

  strFmt(buffer, "Score: " numberfmt("%.2f") "%%",
         (double)(actual * 100) / (double)total);
  printCentered(buffer->data);

  printf("\n");

  strFmt(buffer, underline("CREDITS"));
  printCentered(buffer->data);
  printf("\n");

  strFmt(buffer, italic("%s"), world->meta.title);
  printCentered(buffer->data);
  strFmt(buffer, "%s", world->meta.author);
  printCentered(buffer->data);
  printf("\n");

  strFmt(buffer, "%s", NAME);
  printCentered(buffer->data);
  strFmt(buffer, underline("%s"), "https://github.com/shikaan/ttyny");
  printCentered(buffer->data);
  printf("\n");
}

void uiClearScreen(void) { puts("\e[1;1H\e[2J"); }

void uiFormatAndPrintOpeningCredits(const world_t *world) {
  uiClearScreen();
  char buffer[1024] = {};

  printf("\n\n\n");
  snprintf(buffer, sizeof(buffer), fg_yellow(italic("%s")), world->meta.title);
  printCentered(buffer);
  printf("\n");
  snprintf(buffer, sizeof(buffer), "by");
  printCentered(buffer);
  snprintf(buffer, sizeof(buffer), bold("%s"), world->meta.author);
  printCentered(buffer);
  printf("\n\n\n");
  snprintf(buffer, sizeof(buffer), "[Press ENTER to continue]");
  printCentered(buffer);
}
