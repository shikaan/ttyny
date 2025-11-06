#include "assets/speckled_band_world.h"
#include "src/buffers.h"
#include "src/dm.h"
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

int main(void) {
  string_t *input cleanup(strDestroy) = strCreate(512);
  string_t *response cleanup(strDestroy) = strCreate(4096);
  string_t *target cleanup(strDestroy) = strCreate(128);
  world_t *world = &speckled_band_world;

  dm_t *dm cleanup(dmDestroy) = dmCreate(world);
  panicif(!dm, "cannot create dm");

  parser_t *parser cleanup(parserDestroy) = parserCreate();
  panicif(!parser, "cannot create parser");

  locations_t *locations cleanup(locationsDestroy) =
      locationsCreate(world->locations->length);
  items_t *items cleanup(itemsDestroy) = itemsCreate(world->items->length);

  screenClear();
  formatWelcomeScreen(response);
  printCommandOutput(response);
  fgetc(stdin);
  screenClear();

  ui_handle_t *loading = loadingStart();

  dmDescribeLocation(dm, world->current_location, response);
  loadingStop(&loading);

  printDescription(response);

  formatLocationChange(response, world->current_location);
  printStateUpdate(response);

  while (1) {
    printPrompt();
    strReadFrom(input, stdin);

    if (input->used == 0)
      continue;

    strClear(response);
    loading = loadingStart();

    operation_t operation;
    parserGetOperation(parser, &operation, input);

    if (operation.type == OPERATION_TYPE_COMMAND) {
      switch (operation.as.command) {
      case COMMAND_TYPE_HELP: {
        formatHelp(response, world);
        loadingStop(&loading);
        printCommandOutput(response);
        break;
      }
      case COMMAND_TYPE_STATUS: {
        formatStatus(response, world);
        loadingStop(&loading);
        printCommandOutput(response);
        break;
      }
      case COMMAND_TYPE_TLDR: {
        formatTldr(response, world);
        loadingStop(&loading);
        printCommandOutput(response);
        break;
      }
      case COMMAND_TYPE_QUIT:
        strFmt(response, "Okay, bye!");
        loadingStop(&loading);
        printCommandOutput(response);
        return 0;
      case COMMAND_TYPE_UNKNOWN:
      case COMMAND_TYPES:
      default: {
        strFmt(response, "Not sure how to do that...");
        loadingStop(&loading);
        printError(response);
        break;
      }
      }
      continue;
    }

    action_type_t action = operation.as.action;

    // Advance turn count only for actions, not for commands
    world->state.turns++;

    item_t *item = NULL;
    location_t *location = NULL;

    itemsClear(items);
    locationsClear(locations);

    transition_result_t transition;

    switch (action) {
    case ACTION_TYPE_MOVE: {
      parserExtractTarget(parser, input, world->current_location->exits, items,
                          &location, &item);

      if (!location) {
        strFmt(response, "You cannot go there!");
        loadingStop(&loading);
        printError(response);
        break;
      }

      world->current_location = location;
      // ignoring error: transition are expected to always succeed only for USE
      objectTransition(&location->object, &transition, action);
      dmDescribeLocation(dm, location, response);

      loadingStop(&loading);
      printDescription(response);
      formatLocationChange(response, world->current_location);
      printStateUpdate(response);
      break;
    }
    case ACTION_TYPE_EXAMINE: {
      itemsCat(items, world->current_location->items);
      itemsCat(items, world->state.inventory);
      parserExtractTarget(parser, input, world->current_location->exits, items,
                          &location, &item);

      if (item) {
        // ignoring error: transitions always succeed only for USE
        objectTransition(&item->object, &transition, action);
        dmDescribeObject(dm, &item->object, response);
        loadingStop(&loading);
        printDescription(response);
        break;
      }

      if (location) {
        // ignoring error: transitions always succeed only for USE
        objectTransition(&location->object, &transition, action);
        dmDescribeObject(dm, &location->object, response);
        loadingStop(&loading);
        printDescription(response);
        break;
      }

      loadingStop(&loading);
      strFmt(response, "I don't understand... Can you rephrase that?");
      printError(response);
      break;
    }
    case ACTION_TYPE_TAKE: {
      parserExtractTarget(parser, input, locations,
                          world->current_location->items, &location, &item);

      if (!item) {
        strFmt(response, "Not sure what you want to take.");
        loadingStop(&loading);
        printError(response);
        break;
      }

      if (!objectIsCollectible(&item->object)) {
        strFmt(response, "You cannot pick that up.");
        loadingStop(&loading);
        printError(response);
        break;
      }

      itemsAdd(world->state.inventory, item);
      itemsRemove(world->current_location->items, item);

      dmDescribeSuccess(dm, world, input, response);

      loadingStop(&loading);
      printDescription(response);

      formatTake(response, item);
      printStateUpdate(response);

      // ignoring error: transition are expected to always succeed only for USE
      objectTransition(&item->object, &transition, action);
      break;
    }
    case ACTION_TYPE_DROP: {
      parserExtractTarget(parser, input, locations, world->state.inventory,
                          &location, &item);

      if (!item) {
        strFmt(response, "You cannot drop something that you don't own.");
        loadingStop(&loading);
        printError(response);
        break;
      }

      itemsAdd(world->current_location->items, item);
      itemsRemove(world->state.inventory, item);

      dmDescribeSuccess(dm, world, input, response);
      loadingStop(&loading);

      printDescription(response);
      formatDrop(response, item);
      printStateUpdate(response);

      // ignoring error: transitions always succeed only for USE
      objectTransition(&item->object, &transition, action);
      break;
    }
    case ACTION_TYPE_USE: {
      itemsCat(items, world->state.inventory);
      itemsCat(items, world->current_location->items);
      parserExtractTarget(parser, input, locations, items, &location, &item);

      if (!item) {
        strFmt(response, "Not sure what you mean.");
        loadingStop(&loading);
        printError(response);
        break;
      }

      objectTransition(&item->object, &transition, action);

      if (transition == TRANSITION_RESULT_OK) {
        dmDescribeSuccess(dm, world, input, response);
        dmForget(dm, &item->object);
        loadingStop(&loading);

        printDescription(response);
        formatUse(response, item);
        printStateUpdate(response);
      } else {
        strFmt(response, "Did you mean %s? Unfortunately, it cannot be used...",
               item->object.name);
        loadingStop(&loading);
        printError(response);
      }
      break;
    }
    case ACTION_TYPES:
    case ACTION_TYPE_UNKNOWN:
    default:
      strFmt(response, "Not sure how to do that...");
      loadingStop(&loading);
      printError(response);
    }

    game_state_t game_state = world->digest(&world->state);
    if (game_state != GAME_STATE_CONTINUE) {
      loading = loadingStart();
      dmDescribeEndGame(dm, world, game_state, response);
      loadingStop(&loading);
      printEndGame(response, game_state);
      return 0;
    }
  }

  return 0;
}
