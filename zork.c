#include "assets/ancient_portal.h"
#include "src/buffers.h"
#include "src/narrator.h"
#include "src/panic.h"
#include "src/parser.h"
#include "src/ui.h"
#include "src/utils.h"
#include "src/world.h"
#include <ggml.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *STATE_UPDATE = "\n~>";
const char *NAME = "Zork";

int main(void) {
  string_t *input cleanup(strDestroy) = strCreate(512);
  string_t *response cleanup(strDestroy) = strCreate(4096);
  string_t *target cleanup(strDestroy) = strCreate(128);
  string_t *suggestion cleanup(strDestroy) = strCreate(512);
  world_t *world = &ancient_portal_world;

  narrator_t *narrator cleanup(narratorDestroy) = narratorCreate();
  panicif(!narrator, "cannot create narrator");

  parser_t *parser cleanup(parserDestroy) = parserCreate();
  panicif(!parser, "cannot create narrator");

  puts("\e[1;1H\e[2J");
  strFmt(response,
         " ~   Welcome to %s!\n"
         " ~ \n"
         " ~   You're about to explore a fantasy world through conversation.\n"
         " ~   The game will describe where you are, and you respond in your "
         "own words.\n"
         " ~ \n"
         " ~   Just type naturally, like:\n"
         " ~     • 'I want to go in the kitchen'\n"
         " ~     • 'Pick up the lamp'\n"
         " ~     • 'Can I open this door?'\n"
         " ~ \n"
         " ~   Type '/help' anytime, if you're stuck.\n"
         " ~ \n"
         " ~   [Press ENTER to begin your adventure]",
         NAME);
  printResponse(response);
  fgetc(stdin);
  puts("\e[1;1H\e[2J");

  ui_handle_t *loading = loadingStart();
  narratorDescribeWorld(narrator, world, response);
  strFmtAppend(response, "%s Location: %s.", STATE_UPDATE,
               world->current_location->object.name);
  loadingStop(&loading);
  printResponse(response);

  while (1) {
    printf("> ");
    strReadFrom(input, stdin);

    if (input->used == 0)
      continue;

    world->state.turns++;

    strClear(response);
    loading = loadingStart();

    action_t action = parserExtractAction(parser, input);

    item_t *item = NULL;
    location_t *location = NULL;

    const locations_t *locations = NULL;
    const items_t *items = NULL;

    switch (action) {
    case ACTION_MOVE: {
      debug("action: move");
      items = &NO_ITEMS;
      locations = world->current_location->exits;
      parserExtractTarget(parser, input, locations, items, &location, &item);

      if (!location) {
        narratorCommentFailure(narrator, FAILURE_INVALID_TARGET, input,
                               response);
        goto print;
      }

      world->current_location = location;
      objectTransition(&location->object, action);
      narratorDescribeWorld(narrator, world, response);
      strFmtAppend(response, "%s Location: %s.", STATE_UPDATE,
                   world->current_location->object.name);
      goto print;
    }
    case ACTION_EXAMINE: {
      debug("action: examine\n");
      // TODO: how to examine the inventory?
      items = world->current_location->items;
      locations = world->current_location->exits;
      parserExtractTarget(parser, input, locations, items, &location, &item);

      if (item) {
        objectTransition(&item->object, action);
        narratorDescribeObject(narrator, &item->object, response);
        goto print;
      }

      if (location) {
        objectTransition(&location->object, action);
        narratorDescribeObject(narrator, &location->object, response);
        goto print;
      }

      narratorCommentFailure(narrator, FAILURE_INVALID_TARGET, input, response);
      goto print;
    }
    case ACTION_TAKE: {
      debug("action: take\n");
      items = world->current_location->items;
      locations = &NO_LOCATIONS;
      parserExtractTarget(parser, input, locations, items, &location, &item);

      if (!item) {
        narratorCommentFailure(narrator, FAILURE_INVALID_ITEM, input, response);
        goto print;
      }

      if (!objectIsCollectible(&item->object)) {
        narratorCommentFailure(narrator, FAILURE_CANNOT_COLLECT_ITEM, input,
                               response);
        goto print;
      }

      itemsAdd(world->state.inventory, item);
      itemsRemove(world->current_location->items, item);
      narratorCommentSuccess(narrator, world, input, response);
      strFmtAppend(response, "%s %s taken.", STATE_UPDATE, item->object.name);
      objectTransition(&item->object, action);
      goto print;
    }
    case ACTION_DROP: {
      debug("action: drop\n");
      items = world->state.inventory;
      locations = &NO_LOCATIONS;
      parserExtractTarget(parser, input, locations, items, &location, &item);

      if (!item) {
        narratorCommentFailure(narrator, FAILURE_INVALID_ITEM, input, response);
        goto print;
      }

      itemsAdd(world->current_location->items, item);
      itemsRemove(world->state.inventory, item);
      narratorCommentSuccess(narrator, world, input, response);
      strFmtAppend(response, "%s %s dropped.", STATE_UPDATE, item->object.name);
      objectTransition(&item->object, action);
      goto print;
    }
    case ACTION_USE: {
      debug("action: use\n");
      items = world->state.inventory;
      locations = &NO_LOCATIONS;
      parserExtractTarget(parser, input, locations, items, &location, &item);

      if (!item) {
        narratorCommentFailure(narrator, FAILURE_INVALID_ITEM, input, response);
        goto print;
      }

      narratorCommentSuccess(narrator, world, input, response);
      objectTransition(&item->object, action);
      goto print;
    }
    case ACTION_HELP:
      debug("action: /help\n");
      location_t *first_exit =
          (location_t *)bufAt(world->current_location->exits, 0);

      strFmt(suggestion, "'Go to %s'", first_exit->object.name);

      if (world->current_location->items->used > 0) {
        strFmtAppend(suggestion, " or 'Take %s'",
                     bufAt(world->current_location->items, 0)->object.name);
      }

      strFmt(response,
             " ~  In %s you can explore the world in natural language.\n"
             " ~  \n"
             " ~  When items or locations are described, try interact:\n"
             " ~     • 'Light the lamp'\n"
             " ~     • 'Go to the garden'\n"
             " ~  \n"
             " ~  When an action triggers a change, you will see a message "
             "prefixed with '~>'.\n"
             " ~  \n"
             " ~  Commands are prefixed with a '/' and their output is "
             "prefixed with `~`.\n"
             " ~  Available commands:\n"
             " ~     • '/status' - shows the player status\n"
             " ~     • '/help'   - displays this help\n"
             " ~     • '/quit'   - ends the game\n"
             " ~  \n"
             " ~  Based on your last input, you could try %s.",
             NAME, suggestion->data);
      goto print;
    case ACTION_STATUS: {
      debug("action: /status\n");
      items_t *inventory = world->state.inventory;

      strFmt(response, " ~   Location:  %s\n ~   Turns:     %d\n ~   Inventory:",
             world->current_location->object.name, world->state.turns);
      if (inventory->used == 0) {
        strFmtAppend(response, " empty.");
      } else {
        for (size_t i = 0; i < inventory->used; i++) {
          strFmtAppend(response, "\n ~     • %s",
                       bufAt(inventory, i)->object.name);
        }
      }

      goto print;
    }
    case ACTION_QUIT:
      return 0;
    case ACTIONS:
    case ACTION_UNKNOWN:
    default:
      debug("action: unknown\n");
      strFmt(response, "Not sure how to do that.");
      goto print;
    }

  print:
    loadingStop(&loading);
    printResponse(response);

    switch (world->digest(&world->state)) {
    case GAME_STATE_VICTORY:
      puts("You won!");
      return EXIT_SUCCESS;
    case GAME_STATE_DEAD:
      puts("You're dead!");
      return EXIT_FAILURE;
    case GAME_STATE_CONTINUE:
    default:
      break;
    };
  }

  return 0;
}
