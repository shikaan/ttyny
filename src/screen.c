#include "screen.h"
#include "alloc.h"
#include "buffers.h"
#include "tty.h"
#include "utils.h"
#include "world.h"
#include <stddef.h>
#include <stdio.h>

#define prompt fg_yellow
#define command fg_blue
#define location fg_magenta
#define item fg_cyan

const char *NAME = fg_green(bold("mystty"));

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

void printError(string_t *response) { printResponse(response, fg_red(" !  ")); }

void printStateUpdate(string_t *response) {
  puts("");
  printResponse(response, " ~> ");
}

void printCommandOutput(string_t *response) {
  printResponse(response, " ~   ");
}

void printDescription(string_t *response) { printResponse(response, " |  "); }

void printEndGame(string_t *response, game_state_t state) {
  if (state == GAME_STATE_VICTORY) {
    strFmtAppend(response, fg_green("\n\n     >> You won! <<"))
  } else {
    strFmtAppend(response, fg_red("\n\n     >> You lost! <<"))
  }
  printResponse(response, "    ");
}

void printPrompt(void) { printf("> "); }

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
         NAME, prompt("I want to go in the kitchen"),
         prompt("Pick up the lamp"), prompt("Open this door"),
         command("/help"));
}

void formatLocationChange(string_t *response, location_t *l) {
  strFmt(response, "Location: " location("%s"), l->object.name);
}

void formatTake(string_t *response, const item_t *i) {
  strFmt(response, item("%s") " added to inventory", i->object.name);
}

void formatDrop(string_t *response, const item_t *i) {
  strFmt(response, item("%s") " removed from inventory", i->object.name);
}

void formatUse(string_t *response, const item_t *i) {
  strFmt(response, item("%s") " used", i->object.name);
}

void formatHelp(string_t *response, const world_t *world) {
  string_t *suggestion cleanup(strDestroy) = strCreate(512);

  location_t *first_exit =
      (location_t *)bufAt(world->current_location->exits, 0);

  strFmt(suggestion, prompt("Go to %s"), first_exit->object.name);

  if (world->current_location->items->used > 0) {
    object_t room_item = bufAt(world->current_location->items, 0)->object;
    if (objectIsCollectible(&room_item)) {
      strFmtAppend(suggestion, " or " prompt("Take %s"), room_item.name);
    } else {
      strFmtAppend(suggestion, " or " prompt("Examine %s"), room_item.name);
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
         NAME, prompt("Light the lamp"), prompt("Go to the garden"),
         command("/status"), command("/help"), command("/tldr"),
         command("/quit"), suggestion->data);
}

void formatStatus(string_t *response, const world_t *world) {
  items_t *inventory = world->state.inventory;

  strFmt(response,
         "Location:  " location("%s") "\n"
                                      "Turns:     %d\n"
                                      "Inventory:",
         world->current_location->object.name, world->state.turns);

  if (inventory->used == 0) {
    strFmtAppend(response, dim(" empty") ".");
  } else {
    for (size_t i = 0; i < inventory->used; i++) {
      item_t *inv_item = bufAt(inventory, i);
      strFmtAppend(response, "\n  • " item("%s") " [%s]", inv_item->object.name,
                   bufAt(inv_item->object.state_descriptions,
                         inv_item->object.current_state));
    }
  }
}

void formatTldr(string_t *response, const world_t *world) {
  items_t *room_items = world->current_location->items;
  locations_t *room_exits = world->current_location->exits;

  strFmt(response,
         "Current Location: " location("%s") "\n"
                                             "Items:",
         world->current_location->object.name);

  if (room_items->used == 0) {
    strFmtAppend(response, dim(" none") ".");
  } else {
    for (size_t i = 0; i < room_items->used; i++) {
      item_t *inv_item = bufAt(room_items, i);
      strFmtAppend(response, "\n  • " item("%s"), inv_item->object.name);
    }
  }

  strFmtAppend(response, "\nExits:");
  for (size_t i = 0; i < room_exits->used; i++) {
    location_t *room_exit = (location_t *)bufAt(room_exits, i);
    strFmtAppend(response, "\n  • " location("%s"), room_exit->object.name);
  }
}
