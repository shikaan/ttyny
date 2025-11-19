#include "assets/story_world.h"
#include "src/buffers.h"
#include "src/master.h"
#include "src/panic.h"
#include "src/parser.h"
#include "src/screen.h"
#include "src/utils.h"
#include "src/world.h"
#include <ggml.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void (*print_callback_t)(string_t *);

int main(void) {
  story_world_init(&story_world);
  world_t *world = &story_world;

  string_t *input cleanup(strDestroy) = strCreate(512);
  string_t *response cleanup(strDestroy) = strCreate(4096);
  string_t *state cleanup(strDestroy) = strCreate(1024);
  string_t *target cleanup(strDestroy) = strCreate(128);

  master_t *master cleanup(masterDestroy) = masterCreate(world);
  panicif(!master, "cannot create master");

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

  masterDescribeLocation(master, world->current_location, response);
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
    strClear(state);

    transition_result_t transition;
    print_callback_t printCallback = printDescription;

    switch (action) {
    case ACTION_TYPE_MOVE: {
      parserExtractTarget(parser, input, world->current_location->exits, items,
                          &location, &item);

      if (!location) {
        strFmt(response, "You cannot go there!");
        printCallback = printError;
        break;
      }

      objectTransition(&location->object, action, world->state.inventory,
                       &transition);

      if (transition == TRANSITION_RESULT_MISSING_ITEM) {
        strFmt(response, "You need an item or a key to go there...");
        printCallback = printError;
        break;
      }

      world->current_location = location;
      masterDescribeLocation(master, location, response);
      printCallback = printDescription;
      formatLocationChange(state, world->current_location);
      break;
    }
    case ACTION_TYPE_EXAMINE: {
      itemsCat(items, world->current_location->items);
      itemsCat(items, world->state.inventory);
      parserExtractTarget(parser, input, world->current_location->exits, items,
                          &location, &item);

      if (item) {
        // ignoring error: transitions always succeed only for USE
        objectTransition(&item->object, action, world->state.inventory,
                         &transition);
        masterDescribeObject(master, &item->object, response);
        printCallback = printDescription;
        break;
      }

      if (location) {
        // ignoring error: transitions always succeed only for USE
        objectTransition(&location->object, action, world->state.inventory,
                         &transition);
        masterDescribeObject(master, &location->object, response);
        printCallback = printDescription;
        break;
      }

      strFmt(response, "I don't understand... Can you rephrase that?");
      printCallback = printError;
      break;
    }
    case ACTION_TYPE_TAKE: {
      parserExtractTarget(parser, input, locations,
                          world->current_location->items, &location, &item);

      if (!item) {
        strFmt(response, "Not sure what you want to take.");
        printCallback = printError;
        break;
      }

      if (!item->collectible) {
        strFmt(response, "You cannot pick that up.");
        printCallback = printError;
        break;
      }

      itemsAdd(world->state.inventory, item);
      itemsRemove(world->current_location->items, item);

      masterDescribeSuccess(master, world, input, response);

      printCallback = printDescription;
      formatTake(state, item);

      // ignoring error: transition are expected to always succeed only for USE
      objectTransition(&item->object, action, world->state.inventory,
                       &transition);
      break;
    }
    case ACTION_TYPE_DROP: {
      parserExtractTarget(parser, input, locations, world->state.inventory,
                          &location, &item);

      if (!item) {
        strFmt(response, "You cannot drop something that you don't own.");
        printCallback = printError;
        break;
      }

      itemsAdd(world->current_location->items, item);
      itemsRemove(world->state.inventory, item);

      masterDescribeSuccess(master, world, input, response);

      printCallback = printDescription;
      formatDrop(state, item);

      // ignoring error: transitions always succeed only for USE
      objectTransition(&item->object, action, world->state.inventory,
                       &transition);
      break;
    }
    case ACTION_TYPE_USE: {
      itemsCat(items, world->state.inventory);
      itemsCat(items, world->current_location->items);
      parserExtractTarget(parser, input, locations, items, &location, &item);

      if (!item) {
        strFmt(response, "Not sure what you mean.");
        printCallback = printError;
        break;
      }

      objectTransition(&item->object, action, world->state.inventory,
                       &transition);

      switch (transition) {
      case TRANSITION_RESULT_OK:
        masterDescribeSuccess(master, world, input, response);
        masterForget(master, &item->object);

        printCallback = printDescription;
        formatUse(state, item);
        break;
      case TRANSITION_RESULT_MISSING_ITEM:
        strFmt(response, "You need a utensil for that");
        printCallback = printError;
        break;
      case TRANSITION_RESULT_NO_TRANSITION:
      default:
        strFmt(response, "Did you mean %s? Unfortunately, it cannot be used...",
               item->object.name);
        printCallback = printError;
        break;
      }
      break;
    }
    case ACTION_TYPES:
    case ACTION_TYPE_UNKNOWN:
    default:
      strFmt(response, "Not sure how to do that...");
      printCallback = printError;
    }

    game_state_t game_state = world->digest(world);
    if (game_state != GAME_STATE_CONTINUE) {
      masterDescribeEndGame(master, input, world, game_state, response);
      loadingStop(&loading);
      printEndGame(response, game_state);
      return 0;
    }

    loadingStop(&loading);
    printCallback(response);
    if (state->used > 0) {
      printStateUpdate(state);
    }
  }

  return 0;
}
