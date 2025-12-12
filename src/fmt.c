#include "fmt.h"
#include "lib/buffers.h"
#include "lib/tty.h"
#include "utils.h"
#include "world/item.h"
#include "world/location.h"
#include "world/object.h"
#include "world/world.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>

void fmtWelcomeScreen(string_t *response) {
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

void fmtLocationChange(string_t *response, location_t *l) {
  strFmt(response, "Location: " locationfmt("%s"), l->object.name);
}

void fmtTake(string_t *response, const item_t *i) {
  strFmt(response, itemfmt("%s") " added to inventory", i->object.name);
}

void fmtDrop(string_t *response, const item_t *i) {
  strFmt(response, itemfmt("%s") " removed from inventory", i->object.name);
}

void fmtUse(string_t *response, const item_t *i) {
  strFmt(response, itemfmt("%s") " used", i->object.name);
}

void fmtTransition(string_t *response, const object_t *o) {
  const char *fmt = o->type == OBJECT_TYPE_ITEM ? itemfmt("%s") " changed"
                                                : locationfmt("%s") " changed";
  strFmt(response, fmt, o->name);
}

void fmtHelp(string_t *response, const world_t *world) {
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

void fmtStatus(string_t *response, const world_t *world) {
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

void fmtTldr(string_t *response, const world_t *world) {
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

void fmtCapitalizeWorldObjects(string_t * response, const world_t * world) {
  size_t i = 0;
  bufEach(world->items, i) {
    item_t* item = bufAt(world->items, i);
    strCaseReplace(response, item->object.name, item->object.name);
  }

  bufEach(world->locations, i) {
    location_t* location = bufAt(world->locations, i);
    strCaseReplace(response, location->object.name, location->object.name);
  }
}
