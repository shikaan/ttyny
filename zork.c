#include "assets/urban_escape.h"
#include "src/buffers.h"
#include "src/narrator.h"
#include "src/panic.h"
#include "src/parser.h"
#include "src/screen.h"
#include "src/tty.h"
#include "src/utils.h"
#include "src/world.h"
#include <ggml.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *NAME = fg_green(bold("mystty"));

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

  locations_t *locations cleanup(locationsDestroy) =
      locationsCreate(world->locations->length);
  items_t *items cleanup(itemsDestroy) = itemsCreate(world->items->length);

  printWelcomeScreen(NAME, response);

  ui_handle_t *loading = loadingStart();
  narratorDescribeWorld(narrator, world, response);
  loadingStop(&loading);
  printDescription(response);
  printLocationChange(response, world->current_location);

  while (1) {
    printPrompt();
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
        loadingStop(&loading);
        printError(response);
        break;
      }

      world->current_location = location;
      // ignoring error: transition are expected to always succeed only for USE
      objectTransition(&location->object, &transition, action);
      narratorDescribeWorld(narrator, world, response);

      loadingStop(&loading);
      printDescription(response);
      printLocationChange(response, world->current_location);
      break;
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
        loadingStop(&loading);
        printDescription(response);
        break;
      }

      if (location) {
        // ignoring error: transitions always succeed only for USE
        objectTransition(&location->object, &transition, action);
        narratorDescribeObject(narrator, &location->object, response);
        loadingStop(&loading);
        printDescription(response);
        break;
      }

      loadingStop(&loading);
      narratorCommentFailure(narrator, FAILURE_TYPE_INVALID_TARGET, input,
                             response);
      printError(response);
    }
    case ACTION_TYPE_TAKE: {
      debug("action: take\n");
      parserExtractTarget(parser, input, locations,
                          world->current_location->items, &location, &item);

      if (!item) {
        narratorCommentFailure(narrator, FAILURE_TYPE_INVALID_ITEM, input,
                               response);
        loadingStop(&loading);
        printError(response);
        break;
      }

      if (!objectIsCollectible(&item->object)) {
        narratorCommentFailure(narrator, FAILURE_TYPE_CANNOT_COLLECT_ITEM,
                               input, response);
        loadingStop(&loading);
        printError(response);
        break;
      }

      itemsAdd(world->state.inventory, item);
      itemsRemove(world->current_location->items, item);

      narratorCommentSuccess(narrator, world, input, response);

      loadingStop(&loading);
      printDescription(response);
      strFmt(response, item("%s") " added to inventory.", item->object.name);
      printStateUpdate(response);

      // ignoring error: transition are expected to always succeed only for USE
      objectTransition(&item->object, &transition, action);
      break;
    }
    case ACTION_TYPE_DROP: {
      debug("action: drop\n");
      parserExtractTarget(parser, input, locations, world->state.inventory,
                          &location, &item);

      if (!item) {
        narratorCommentFailure(narrator, FAILURE_TYPE_INVALID_ITEM, input,
                               response);
        loadingStop(&loading);
        printError(response);
        break;
      }

      itemsAdd(world->current_location->items, item);
      itemsRemove(world->state.inventory, item);

      narratorCommentSuccess(narrator, world, input, response);
      loadingStop(&loading);

      printDescription(response);
      strFmt(response, item("%s") " removed from inventory.",
             item->object.name);
      printStateUpdate(response);

      // ignoring error: transitions always succeed only for USE
      objectTransition(&item->object, &transition, action);
      break;
    }
    case ACTION_TYPE_USE: {
      debug("action: use\n");
      itemsCat(items, world->state.inventory);
      itemsCat(items, world->current_location->items);
      parserExtractTarget(parser, input, locations, items, &location, &item);

      if (!item) {
        narratorCommentFailure(narrator, FAILURE_TYPE_INVALID_ITEM, input,
                               response);
        loadingStop(&loading);
        printError(response);
        break;
      }

      objectTransition(&item->object, &transition, action);

      if (transition == TRANSITION_RESULT_OK) {
        narratorCommentSuccess(narrator, world, input, response);
        loadingStop(&loading);

        printDescription(response);
        strFmt(response, item("%s") " used.", item->object.name);
        printStateUpdate(response);
      } else {
        narratorCommentFailure(narrator, FAILURE_TYPE_CANNOT_BE_USED, input,
                               response);
        loadingStop(&loading);
        printError(response);
      }
      break;
    }
    case ACTION_TYPE_HELP: {
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
             "In %s you can explore the world in natural language.\n"
             "\n"
             "When items or locations are described, try to interact:\n"
             "   • %s\n"
             "   • %s\n"
             "\n"
             "When an action triggers a change, you will see a message "
             "prefixed with '~>'.\n"
             "You can type commands too. They all start with '/' and their "
             "output is prefixed with `~`.\n"
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
      loadingStop(&loading);
      printCommandOutput(response);
      break;
    }
    case ACTION_TYPE_STATUS: {
      debug("action: /status\n");
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
          strFmtAppend(response, "\n  • " item("%s") " [%s]",
                       inv_item->object.name,
                       bufAt(inv_item->object.state_descriptions,
                             inv_item->object.current_state));
        }
      }

      loadingStop(&loading);
      printCommandOutput(response);
      break;
    }
    case ACTION_TYPE_TLDR: {
      debug("action: /tldr\n");
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

      loadingStop(&loading);
      printCommandOutput(response);
      break;
    }
    case ACTION_TYPE_QUIT:
      debug("action: quit\n");
      return 0;
    case ACTION_TYPES:
    case ACTION_TYPE_UNKNOWN:
    default:
      debug("action: unknown\n");
      strFmt(response, "Not sure how to do that.");
      loadingStop(&loading);
      printError(response);
    }

    game_state_t game_state = world->digest(&world->state);
    if (game_state != GAME_STATE_CONTINUE) {
      narratorDescribeEndGame(narrator, world, game_state, response);
      loadingStop(&loading);
      printDescription(response);
      return 0;
    }
  }

  return 0;
}
