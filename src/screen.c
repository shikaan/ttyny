#include "screen.h"
#include "lib/alloc.h"
#include "lib/buffers.h"
#include "lib/set.h"
#include "lib/tty.h"
#include "utils.h"
#include "world/item.h"
#include "world/location.h"
#include "world/object.h"
#include "world/world.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>

#define promptfmt fg_yellow
#define commandfmt fg_blue
#define locationfmt fg_magenta
#define itemfmt fg_cyan
#define successfmt fg_green
#define failfmt fg_red
#define descriptionfmt italic
#define numberfmt bold

const char *NAME = fg_green(bold("ttyny"));

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
  if (!(*handle) || !handle)
    return;
  (*handle)->stop = 1;
  pthread_join((*handle)->tid, NULL);
  deallocate(handle);
}

static const int max_line_len = 80;
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
    if (col > prefix_len && col + word_len > max_line_len) {
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
  size_t prefix_len = max_line_len / 2 - strVisibleLength(str) / 2;

  if (prefix_len < max_line_len) {
    for (size_t i = 0; i < prefix_len; i++) {
      putchar(' ');
    }
  }

  printf("%s\n", str);
}

void printError(string_t *response) {
  printResponse(response, failfmt(" !  "));
}

void printStateUpdate(string_t *response) { printResponse(response, "\n ~> "); }

void printCommandOutput(string_t *response) { printResponse(response, " ~  "); }

void printDescription(string_t *response) { printResponse(response, " |  "); }

void printReadable(string_t *response) {
  printResponse(response, "    " ESC_ITALIC);
}

void printEndGame(string_t *buffer, game_state_t state, const world_t *world) {
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
         world->items->used);
  printCentered(buffer->data);

  size_t discovered_locations = setUsed(world->discovered_locations);
  strFmt(buffer, "Locations: " numberfmt("%lu/%lu"), discovered_locations,
         world->locations->used);
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
    if (transitions && transitions->used > 0) {
      total_puzzles++;
    }
  }

  size_t solved_puzzles = setUsed(world->solved_puzzles);
  strFmt(buffer, "Puzzles: " numberfmt("%lu/%lu"), solved_puzzles,
         total_puzzles);
  printCentered(buffer->data);

  size_t actual = discovered_locations + discovered_items + solved_puzzles;
  size_t total = total_puzzles + world->locations->used + world->items->used;

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

void screenClear(void) { puts("\e[1;1H\e[2J"); }

void formatWelcomeScreen(string_t *response) {
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
         NAME, promptfmt("I want to go in the kitchen"),
         promptfmt("Pick up the lamp"), promptfmt("Open this door"),
         commandfmt("/help"));
}

void printOpeningCredits(const world_t *world) {
  screenClear();
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

void formatLocationChange(string_t *response, location_t *l) {
  strFmt(response, "Location: " locationfmt("%s"), l->object.name);
}

void formatTake(string_t *response, const item_t *i) {
  strFmt(response, itemfmt("%s") " added to inventory", i->object.name);
}

void formatDrop(string_t *response, const item_t *i) {
  strFmt(response, itemfmt("%s") " removed from inventory", i->object.name);
}

void formatUse(string_t *response, const item_t *i) {
  strFmt(response, itemfmt("%s") " used", i->object.name);
}

void formatHelp(string_t *response, const world_t *world) {
  string_t *suggestion cleanup(strDestroy) = strCreate(512);

  location_t *first_exit = (location_t *)bufAt(world->location->exits, 0);

  strFmt(suggestion, promptfmt("Go to %s"), first_exit->object.name);

  if (!bufIsEmpty(world->location->items)) {
    item_t *room_item = bufAt(world->location->items, 0);
    if (room_item->collectible) {
      strFmtAppend(suggestion, " or " promptfmt("Take %s"),
                   room_item->object.name);
    } else {
      strFmtAppend(suggestion, " or " promptfmt("Examine %s"),
                   room_item->object.name);
    }
  }

  strFmt(response,
         "In %s you can explore the world in natural language.\n"
         "\n"
         "When items or locations are described, try to interact:\n"
         "   • %s\n"
         "   • %s\n"
         "\n"
         "Changes to the environment are prefixed with '~>'.\n"
         "You can type commands too! Their output is prefixed with `~`.\n"
         "Available commands:\n"
         "   • %s - shows the player status\n"
         "   • %s   - displays this help\n"
         "   • %s   - summarizes the current location\n"
         "   • %s   - ends the game\n"
         "\n"
         "Based on your last input, you could try %s.",
         NAME, promptfmt("Light the lamp"), promptfmt("Go to the garden"),
         commandfmt("/status"), commandfmt("/help"), commandfmt("/tldr"),
         commandfmt("/quit"), suggestion->data);
}

void formatStatus(string_t *response, const world_t *world) {
  items_t *inventory = world->inventory;

  strFmt(response,
         "Location:  " locationfmt("%s") "\n"
                                         "Turns:     %d\n"
                                         "Inventory:",
         world->location->object.name, world->turns);

  if (bufIsEmpty(inventory)) {
    strFmtAppend(response, dim(" empty"));
  } else {
    size_t i = 0;
    bufEach(inventory, i) {
      item_t *inv_item = bufAt(inventory, i);
      strFmtAppend(response, "\n  • " itemfmt("%s"), inv_item->object.name);
    }
  }
}

void formatTldr(string_t *response, const world_t *world) {
  items_t *room_items = world->location->items;
  locations_t *room_exits = world->location->exits;

  strFmt(response,
         "Current Location: " locationfmt("%s") "\n"
                                                "Items:",
         world->location->object.name);

  if (bufIsEmpty(room_items)) {
    strFmtAppend(response, dim(" none") ".");
  } else {
    size_t i = 0;
    bufEach(room_items, i) {
      item_t *inv_item = bufAt(room_items, i);
      strFmtAppend(response, "\n  • " itemfmt("%s"), inv_item->object.name);
    }
  }

  strFmtAppend(response, "\nExits:");
  size_t i = 0;
  bufEach(room_exits, i) {
    location_t *room_exit = (location_t *)bufAt(room_exits, i);
    strFmtAppend(response, "\n  • " locationfmt("%s"), room_exit->object.name);
  }
}
