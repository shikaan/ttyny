#include "assets/urban_escape.h"
#include "src/buffers.h"
#include "src/narrator.h"
#include "src/panic.h"
#include "src/parser.h"
#include "src/tty.h"
#include "src/ui.h"
#include "src/utils.h"
#include "src/world.h"
#include <ggml.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *STATE_UPDATE = "\n~>";
const char *NAME = fg_green(bold("mystty"));

#define prompt fg_yellow
#define command fg_blue
#define location fg_magenta
#define item fg_cyan

int main(void) {
  string_t *input cleanup(strDestroy) = strCreate(512);
  string_t *response cleanup(strDestroy) = strCreate(4096);
  string_t *target cleanup(strDestroy) = strCreate(128);
  string_t *suggestion cleanup(strDestroy) = strCreate(512);
  world_t *world = &urban_escape_world;

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
         " ~   Just type one action at a time naturally, like:\n"
         " ~     • %s\n"
         " ~     • %s\n"
         " ~     • %s\n"
         " ~ \n"
         " ~   Type %s anytime, if you're stuck.\n"
         " ~ \n"
         " ~   [Press ENTER to begin your adventure]",
         NAME, prompt("I want to go in the kitchen"),
         prompt("Pick up the lamp"), prompt("Open this door"),
         command("/help"));
  printResponse(response);
  fgetc(stdin);
  puts("\e[1;1H\e[2J");

  ui_handle_t *loading = loadingStart();
  narratorDescribeWorld(narrator, world, response);
  strFmtAppend(response, "%s Location: " location("%s") ".", STATE_UPDATE,
               world->current_location->object.name);
  loadingStop(&loading);
  printResponse(response);

  locations_t *locations cleanup(locationsDestroy) =
      locationsCreate(world->locations->length);
  items_t *items cleanup(itemsDestroy) = itemsCreate(world->items->length);

  while (1) {
    printf("> ");
    strReadFrom(input, stdin);

    if (input->used == 0)
      continue;

    world->state.turns++;

    strClear(response);
    loading = loadingStart();

    action_type_t action = parserExtractAction(parser, input);

    item_t *item = NULL;
    location_t *location = NULL;

    itemsClear(items);
    locationsClear(locations);

    transition_result_t transition;

    switch (action) {
    case ACTION_TYPE_MOVE: {
      debug("action: move");
      parserExtractTarget(parser, input, world->current_location->exits, items,
                          &location, &item);

      if (!location) {
        narratorCommentFailure(narrator, FAILURE_TYPE_INVALID_TARGET, input,
                               response);
        goto print;
      }

      world->current_location = location;
      // ignoring error: transition are expected to always succeed only for USE
      objectTransition(&location->object, &transition, action);
      narratorDescribeWorld(narrator, world, response);
      strFmtAppend(response, "%s Location: " location("%s") ".", STATE_UPDATE,
                   world->current_location->object.name);
      goto print;
    }
    case ACTION_TYPE_EXAMINE: {
      debug("action: examine\n");
      itemsCat(items, world->current_location->items);
      itemsCat(items, world->state.inventory);
      parserExtractTarget(parser, input, world->current_location->exits, items,
                          &location, &item);

      if (item) {
        // ignoring error: transitions always succeed only for USE
        objectTransition(&item->object, &transition, action);
        narratorDescribeObject(narrator, &item->object, response);
        goto print;
      }

      if (location) {
        // ignoring error: transitions always succeed only for USE
        objectTransition(&location->object, &transition, action);
        narratorDescribeObject(narrator, &location->object, response);
        goto print;
      }

      narratorCommentFailure(narrator, FAILURE_TYPE_INVALID_TARGET, input,
                             response);
      goto print;
    }
    case ACTION_TYPE_TAKE: {
      debug("action: take\n");
      parserExtractTarget(parser, input, locations,
                          world->current_location->items, &location, &item);

      if (!item) {
        narratorCommentFailure(narrator, FAILURE_TYPE_INVALID_ITEM, input,
                               response);
        goto print;
      }

      if (!objectIsCollectible(&item->object)) {
        narratorCommentFailure(narrator, FAILURE_TYPE_CANNOT_COLLECT_ITEM,
                               input, response);
        goto print;
      }

      itemsAdd(world->state.inventory, item);
      itemsRemove(world->current_location->items, item);
      narratorCommentSuccess(narrator, world, input, response);
      strFmtAppend(response, "%s " item("%s") " added to inventory.",
                   STATE_UPDATE, item->object.name);

      // ignoring error: transition are expected to always succeed only for USE
      objectTransition(&item->object, &transition, action);
      goto print;
    }
    case ACTION_TYPE_DROP: {
      debug("action: drop\n");
      parserExtractTarget(parser, input, locations, world->state.inventory,
                          &location, &item);

      if (!item) {
        narratorCommentFailure(narrator, FAILURE_TYPE_INVALID_ITEM, input,
                               response);
        goto print;
      }

      itemsAdd(world->current_location->items, item);
      itemsRemove(world->state.inventory, item);
      narratorCommentSuccess(narrator, world, input, response);
      strFmtAppend(response, "%s " item("%s") " removed from inventory.",
                   STATE_UPDATE, item->object.name);
      // ignoring error: transitions always succeed only for USE
      objectTransition(&item->object, &transition, action);
      goto print;
    }
    case ACTION_TYPE_USE: {
      debug("action: use\n");
      itemsCat(items, world->state.inventory);
      itemsCat(items, world->current_location->items);
      parserExtractTarget(parser, input, locations, items, &location, &item);

      if (!item) {
        narratorCommentFailure(narrator, FAILURE_TYPE_INVALID_ITEM, input,
                               response);
        goto print;
      }

      objectTransition(&item->object, &transition, action);

      if (transition == TRANSITION_RESULT_OK) {
        narratorCommentSuccess(narrator, world, input, response);
        strFmtAppend(
            response, "%s " item("%s") " used.", STATE_UPDATE,
            item->object.name,
            bufAt(item->object.state_descriptions, item->object.current_state));
      } else {
        narratorCommentFailure(narrator, FAILURE_TYPE_CANNOT_BE_USED, input,
                               response);
      }

      goto print;
    }
    case ACTION_TYPE_HELP:
      debug("action: /help\n");
      location_t *first_exit =
          (location_t *)bufAt(world->current_location->exits, 0);

      strFmt(suggestion, prompt("Go to %s"), first_exit->object.name);

      if (world->current_location->items->used > 0) {
        for (size_t i = 0; i < world->current_location->items->used; i++) {
          object_t room_item = bufAt(world->current_location->items, i)->object;
          if (objectIsCollectible(&room_item)) {
            strFmtAppend(suggestion, " or " prompt("Take %s"), room_item.name);
            break;
          } else {
            strFmtAppend(suggestion, " or " prompt("Examine %s"),
                         room_item.name);
            break;
          }
        }
      }

      strFmt(response,
             " ~  In %s you can explore the world in natural language.\n"
             " ~  \n"
             " ~  When items or locations are described, try to interact:\n"
             " ~     • %s\n"
             " ~     • %s\n"
             " ~  \n"
             " ~  When an action triggers a change, you will see a message "
             "prefixed with '~>'.\n"
             " ~  You can type commands too. They all start with '/' and their "
             "output is prefixed with `~`.\n"
             " ~  Available commands:\n"
             " ~     • %s - shows the player status\n"
             " ~     • %s   - displays this help\n"
             " ~     • %s   - summarizes the current location\n"
             " ~     • %s   - ends the game\n"
             " ~  \n"
             " ~  Based on your last input, you could try %s.",
             NAME, prompt("Light the lamp"), prompt("Go to the garden"),
             command("/status"), command("/help"), command("/tldr"),
             command("/quit"), suggestion->data);
      goto print;
    case ACTION_TYPE_STATUS: {
      debug("action: /status\n");
      items_t *inventory = world->state.inventory;

      strFmt(response,
             " ~   Location:  " location("%s") "\n"
                                               " ~   Turns:     %d\n"
                                               " ~   Inventory:",
             world->current_location->object.name, world->state.turns);

      if (inventory->used == 0) {
        strFmtAppend(response, dim(" empty") ".");
      } else {
        for (size_t i = 0; i < inventory->used; i++) {
          item_t *inv_item = bufAt(inventory, i);
          strFmtAppend(response, "\n ~     • " item("%s") " [%s]",
                       inv_item->object.name,
                       bufAt(inv_item->object.state_descriptions,
                             inv_item->object.current_state));
        }
      }

      goto print;
    }
    case ACTION_TYPE_TLDR: {
      debug("action: /tldr\n");
      items_t *room_items = world->current_location->items;
      locations_t *room_exits = world->current_location->exits;

      strFmt(response,
             " ~   Current Location: " location("%s") "\n"
                                                      " ~   Items:",
             world->current_location->object.name);

      if (room_items->used == 0) {
        strFmtAppend(response, dim(" none") ".");
      } else {
        for (size_t i = 0; i < room_items->used; i++) {
          item_t *inv_item = bufAt(room_items, i);
          strFmtAppend(response, "\n ~     • " item("%s"),
                       inv_item->object.name);
        }
      }

      strFmtAppend(response, "\n ~   Exits:");
      for (size_t i = 0; i < room_exits->used; i++) {
        location_t *room_exit = (location_t *)bufAt(room_exits, i);
        strFmtAppend(response, "\n ~     • " location("%s"),
                     room_exit->object.name);
      }

      goto print;
    }
    case ACTION_TYPE_QUIT:
      debug("action: quit\n");
      return 0;
    case ACTION_TYPES:
    case ACTION_TYPE_UNKNOWN:
    default:
      debug("action: unknown\n");
      strFmt(response, "Not sure how to do that.");
      goto print;
    }

  print:
    loadingStop(&loading);
    printResponse(response);

    game_state_t game_state = world->digest(&world->state);
    if (game_state != GAME_STATE_CONTINUE) {
      narratorDescribeEndGame(narrator, world, game_state, response);
      printResponse(response);
      return 0;
    }
  }

  return 0;
}
